import pyb, sensor, image, time, math
enable_lens_corr = False # turn on for straighter lines...
sensor.reset()
#sensor.set_pixformat(sensor.GRAYSCALE) # grayscale is faster
sensor.set_pixformat(sensor.RGB565) # grayscale is faster
sensor.set_framesize(sensor.QQVGA) #80 * 60
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False)  # must turn this off to prevent image washout...
sensor.set_auto_whitebal(False)  # must turn this off to prevent image washout...
clock = time.clock()
THRESHOLD = (200, 255)
BINARY_VISIBLE = False

uart = pyb.UART(3,9600,timeout_char=1000)
uart.init(9600,bits=8,parity = None, stop=1, timeout_char=1000)

l_x = 160
l_y = 120

f_x = (2.8 / 3.984) * l_x # find_apriltags defaults to this if not set
f_y = (2.8 / 2.952) * l_y # find_apriltags defaults to this if not set
c_x = l_x * 0.5 # find_apriltags defaults to this if not set (the image.w * 0.5)
c_y = l_y * 0.5 # find_apriltags defaults to this if not set (the image.h * 0.5)

# All lines also have `x1()`, `y1()`, `x2()`, and `y2()` methods to get their end-points
# and a `line()` method to get all the above as one 4 value tuple for `draw_line()`.
i = 0
j = 0
while(True):
    clock.tick()
    #img = sensor.snapshot().binary([THRESHOLD]) if BINARY_VISIBLE else sensor.snapshot()
    img = sensor.snapshot()
    if enable_lens_corr: img.lens_corr(1.8) # for 2.8mm lens...

    # `merge_distance` controls the merging of nearby lines. At 0 (the default), no
    # merging is done. At 1, any line 1 pixel away from another is merged... and so
    # on as you increase this value. You may wish to merge lines as line segment
    # detection produces a lot of line segment results.
    for blob in img.find_blobs([(0, 30, 5, 40, 10, 30)], area_threshold = 3500, pixels_threshold = 3500, robust = True):
        img.draw_rectangle(blob.rect(), color = 127)
        j = 1
    # `max_theta_diff` controls the maximum amount of rotation difference between
    # any two lines about to be merged. The default setting allows for 15 degrees.
    #line = img.get_regression([(240, 255) if BINARY_VISIBLE else THRESHOLD], area_threshold = 0, pixels_threshold = 20, robust = True)

    img.to_grayscale()
    line = img.get_regression([THRESHOLD], area_threshold = 20, pixels_threshold = 20, robust = True)

    for tag in img.find_apriltags(fx=f_x, fy=f_y, cx=c_x, cy=c_y):
        #img.draw_rectangle(tag.rect(), color = 127)
        i = 1

    if (line): img.draw_line(line.line(), color = 127)
    if line:
        #print(line.line())
        if (j == 1):
            uart.write(("/Dodge/run %d \r\n" % blob.cx()).encode())
            #print(blob.cx())
        elif (i == 1) and ((-tag.z_translation() * 2.88 - 1.37) > 30):
            #print(tag.cx())
            img.draw_rectangle(tag.rect(), color = 127)
            uart.write(("/Tag/run %d \r\n" % tag.cx()).encode())
            #print(("/Tag/run %d \r\n" % tag.cx()))
        else:
            uart.write(("/Line/run %d %d %d %d\r\n" % line.line()).encode())
            #print(("/Line/run %d %d %d %d\r\n" % line.line()))
        time.sleep_ms(20)
    else :
        if (j == 1):
            uart.write(("/Dodge/run %d \r\n" % blob.cx()).encode())
            #print(blob.cx())
        elif (i == 1) and ((-tag.z_translation() * 2.88 - 1.37) > 30):
            #print(tag.cx())
            img.draw_rectangle(tag.rect(), color = 127)
            uart.write(("/Tag/run %d \r\n" % tag.cx()).encode())
            #print(("/Tag/run %d \r\n" % tag.cx()))
        else:
            uart.write(("/stop/run \r\n").encode())
            #print(("/Line/run %d %d %d %d\r\n" % line.line()))
        time.sleep_ms(20)
    print(clock.fps())

    i = 0
    j = 0
