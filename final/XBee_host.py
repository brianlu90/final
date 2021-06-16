import serial
import time

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600, timeout=3)

print('Start Communication')
while 1:
    char = s.read(1)
    print(char.decode(), end = '')

s.close()