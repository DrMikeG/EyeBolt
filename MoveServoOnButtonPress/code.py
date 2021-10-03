"""CircuitPython Essentials Servo standard servo example"""
import time
import board
import pwmio
import digitalio
from adafruit_motor import servo
from digitalio import DigitalInOut, Direction, Pull

# create a PWMOut object on Pin A2.
pwm = pwmio.PWMOut(board.GP2, duty_cycle=2 ** 15, frequency=50)

# Create a servo object, my_servo.
my_servo = servo.Servo(pwm)

button14 = digitalio.DigitalInOut(board.GP14)
button14.switch_to_input(pull=digitalio.Pull.UP)

button15 = digitalio.DigitalInOut(board.GP15)
button15.switch_to_input(pull=digitalio.Pull.UP)


while True:

    # Wait for button 14 to be pressed (==False)
    while button15.value == True:
        time.sleep(0.1)

    print("moving")
        #my_servo.angle = 105 # up
        #my_servo.angle = 0 # completely out-stretched
        #my_servo.angle = 0 # completely out-stretched

    for angle in range(105, 0, -5):
        my_servo.angle = angle
        time.sleep(0.05)

    for angle in range(0, 105, 5):  # 0 - 180 degrees, 5 degrees at a time.
        my_servo.angle = angle
        time.sleep(0.05)
