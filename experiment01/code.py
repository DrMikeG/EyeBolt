
import time

import board
import busio
import digitalio
import adafruit_vl6180x
from digitalio import DigitalInOut, Direction, Pull
from adafruit_motor import stepper


button14 = digitalio.DigitalInOut(board.GP14)
button14.switch_to_input(pull=digitalio.Pull.UP)

button15 = digitalio.DigitalInOut(board.GP15)
button15.switch_to_input(pull=digitalio.Pull.UP)

i2c = busio.I2C(scl=board.GP1, sda=board.GP0)
sensor = adafruit_vl6180x.VL6180X(i2c)

# Stepper motor setup
DELAY = 0.1  # fastest is ~ 0.004, 0.01 is still very smooth, gets steppy after that
STEPS = 200  # this is a full 360º
coils = (
    DigitalInOut(board.GP21),  # A1
    DigitalInOut(board.GP20),  # A2
    DigitalInOut(board.GP19),  # B1
    DigitalInOut(board.GP18),  # B2
)
for coil in coils:
    coil.direction = Direction.OUTPUT

stepper_motor = stepper.StepperMotor(
    coils[0], coils[1], coils[2], coils[3], microsteps=None
)


while True:
    
    # Wait for button 14 to be pressed (==False)
    while button14.value == True:
        time.sleep(0.1)
 
    print("start experiment")
    for s in range(STEPS):
        # Read the range in millimeters and print it.
        range_mm = sensor.range
        print("Step:\t {0} \tRange:\t {1}".format(s,range_mm))
        stepper_motor.onestep(direction=stepper.FORWARD)
        time.sleep(DELAY)
    
    stepper_motor.release()
        
