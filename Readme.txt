108061120 Final Project
(1) Setup:  
    The left servo is connected to D12 and the right servo is connected to D13.
    Tx and Rx of openMV are connected to D1 and D0.
    Tx and Rx of xbee are connected to D10 and D9.
    ping is connected to D6
    * The main.cpp should run with the bbcar functions provided in the github final repo(/final/bbcar), since some functions in bbcar are modified for this project.

(2) Environment:
    The car should run in the environment where the floor is dark and lines are white.
    Also, remove all the white stuffs that might be recognized as lines.
    You can change the BINARY_VISIBLE to "true" and use grayscale snapshot in Final_openMV.py to check whether the environment is clear for line steering.
    The obstacle should be in red color. However, you might have to measure and change the "LAB" threshold for the obstacle whenever you change the environment.
    The apriltags should be 20cm away from the forked road, and make sure that apriltags are bright enough so that the openMV can have a clear vision.

(3) Run:
    After running the main.cpp and Final_openMV.py, the car will follow the white lines.
    When facing the forked road, the car will take the direction where apriltag is placed.
    When there is an obstacle, the car will measure the distance by ping and bypass the obstacle.
    The car will stop when the line is ended.

