#include "mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"

Ticker servo_ticker;
PwmOut pin5(D12), pin6(D13);
BufferedSerial uart(D1,D0); //tx,rx
BufferedSerial xbee(D10, D9);   //tx,rx
void LineFollow(Arguments *in, Reply *out);
void TagFollow(Arguments *in, Reply *out);
void Dodge(Arguments *in, Reply *out);
void RPC_stop(Arguments *in, Reply *out);
void goStraight_cm(float length);
void turn_90(float fc);
RPCFunction rpcLine(&LineFollow, "Line");
RPCFunction rpcTag(&TagFollow, "Tag");
RPCFunction rpcDodge(&Dodge, "Dodge");
RPCFunction rpcStop(&RPC_stop, "stop");
DigitalInOut ping(D6);
const float slope  = 0.10;
bool L = 0, T = 0, D = 0, S = 0;


BBCar car(pin5, pin6, servo_ticker);
parallax_ping  ping1(ping);

int main() {
    char buf[256], outbuf[256];

    double pwm_table0[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table0[] = {-15.147, -14.49, -11.712, -10.603, -4.863, 0.000, 6.059, 11.400, 14.031, 14.988, 15.368};
    double pwm_table1[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table1[] = {-15.625, -14.988, -14.270, -11.321, -5.501, 0.000, 5.899, 11.480, 14.350, 15.386, 15.785};

    // first and fourth argument : length of table
    car.setCalibTable(11, pwm_table0, speed_table0, 11, pwm_table1, speed_table1);
    uart.set_baud(9600);
    //xbee.set_baud(9600);
    while (1) {
        memset(buf, 0, 256);
        for (int i = 0; i < 256; i++) {
            char *recv = new char[1];
            uart.read(recv, sizeof(recv));
            if (*recv == '\r' || *recv == '\n') {
                break;
            }
            buf[i] = *recv;
            delete [] recv;
        }
        //printf("%s\r\n", buf);
        RPC::call(buf, outbuf);
    }
}

void LineFollow(Arguments *in, Reply *out)
{
    int x1, x2, y1, y2;
    int line_mid;

    x1 = in->getArg<int>();
    y1 = in->getArg<int>();
    x2 = in->getArg<int>();
    y2 = in->getArg<int>();
    line_mid = (x1 + x2) / 2;

    if (L == 0) {
        xbee.write("Line\r\n", 6);
        printf("Line \r\n");
        L = 1;
    }
    D = 0;
    T = 0;
    S = 0;
    //xbee.write("Line \r\n", 7);
    

    //line_mid = x1;
    if (line_mid < 20) {
        car.turn_normp(70, -0.2);
    } else if (line_mid < 60) {
        car.turn_normp(70, -0.5);
    } else if (line_mid < 100) {
        car.goStraightCalib_normp(14);
    } else if (line_mid < 140) {
        car.turn_normp(70, 0.5);
    } else if (line_mid <= 160 ) {
        car.turn_normp(70, 0.2);
    }
    ThisThread::sleep_for(20ms);
    //printf("x1 = %d\r\n", x1);
    return;
}

void TagFollow(Arguments *in, Reply *out)
{
    int cx;

    cx = in->getArg<int>();

    if (T == 0) {
        xbee.write("Fork\r\n", 6);
        printf("Fork \r\n");
        T = 1;
    }
    L = 0;
    D = 0;
    S = 0;
    //xbee.write("Fork \r\n", 7);

    if (cx < 20) {
        car.turn_normp(40, -0.2);
    } else if (cx < 60) {
        car.turn_normp(40, -0.5);
    } else if (cx < 100) {
        car.goStraightCalib_normp(8);
    } else if (cx < 140) {
        car.turn_normp(40, 0.5);
    } else if (cx <= 160 ) {
        car.turn_normp(40, 0.2);
    }
}

void Dodge(Arguments *in, Reply *out)
{
    int cx, dist;

    cx = in->getArg<int>();
    dist = (float)ping1;

    xbee.write("Obstacle Start\r\n", 16);
    printf("OB Start\r\n");
    
    T = 0;
    L = 0;
    D = 0;
    S = 0;

    if (cx < 80) {
        turn_90(0.1);
        goStraight_cm(5);
        turn_90(-0.1);
        goStraight_cm(dist + 7);
        turn_90(-0.1);
        goStraight_cm(5);
        turn_90(0.1);
    } else {
        goStraight_cm(1);
        turn_90(-0.1);
        goStraight_cm(5);
        turn_90(0.1);
        goStraight_cm(dist + 6);
        turn_90(0.1);
        goStraight_cm(5);
        turn_90(-0.1);
    }
    xbee.write("Obstacle End\r\n", 14);
    printf("OB end\r\n");

}

void RPC_stop (Arguments *in, Reply *out)   {

    if (S == 0) {
        xbee.write("Stop\r\n", 6);
        printf("Stop\r\n");
        S = 1;
    }
    L = 0;
    T = 0;
    D = 0;
    
    //xbee.write("Stop \r\n", 7);

    car.stop();
    return;
}

void goStraight_cm(float length) {
    if (length > 0) {
        car.goStraightCalib_normp(10);
    } else {
        car.goStraightCalib_normp(-10);
        length = -length;
    }
    int stop = length * 10 * slope + 8;
    for (int i = 0; i < stop; i++) {
        ThisThread::sleep_for(100ms);
    }
    car.stop_normp();
    ThisThread::sleep_for(200ms);
}

void turn_90(float fc) {
    car.turn_normp(52, fc);
    if (fc > 0) {
        ThisThread::sleep_for(2s);
        ThisThread::sleep_for(400ms);
    } else {
        ThisThread::sleep_for(1s);
        ThisThread::sleep_for(800ms);
    }
    car.stop_normp();
    ThisThread::sleep_for(100ms);
}

