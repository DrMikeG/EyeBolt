# This will be my main script going forward
# Notes on progress:
# all button inputs tested


import time
import board
import digitalio
from digitalio import DigitalInOut, Direction, Pull
from adafruit_motor import stepper


button12 = digitalio.DigitalInOut(board.GP12)
button12.switch_to_input(pull=digitalio.Pull.UP)
button13 = digitalio.DigitalInOut(board.GP13)
button13.switch_to_input(pull=digitalio.Pull.UP)

# Go / OK / start button
button14 = digitalio.DigitalInOut(board.GP14)
button14.switch_to_input(pull=digitalio.Pull.UP)

# Electronic break button - press if the robotics are out of range / control
button15 = digitalio.DigitalInOut(board.GP15)
button15.switch_to_input(pull=digitalio.Pull.UP)

photoSensorPin2 = digitalio.DigitalInOut(board.GP3)
photoSensorPin2.switch_to_input(pull=digitalio.Pull.UP)

led = DigitalInOut(board.LED)
led.direction = Direction.OUTPUT
led.value = True

def blink(times):
    for _ in range(times):
        led.value = False
        time.sleep(0.1)
        led.value = True
        time.sleep(0.1)

def CheckEBreak():
    # Button 15 is the EBreak - if this is pressed then:
    # Send Servo at safe value
    # Wait for OK
    if button15.value == False:
        # Park Servo
        print("PANIC")
        while True:            
            if button14.value == False:
                print("OK button releases panic")
                break
            time.sleep(0.1)


while True:
    
    CheckEBreak()

    if button12.value == False:
        print("You pressed button 12!")
        time.sleep(0.5)
    if button13.value == False:
        print("You pressed button 13!")
        time.sleep(0.5)
    if button14.value == False:
        print("You pressed button 14!")
        time.sleep(0.5)
    if photoSensorPin2.value == True:
        print("Photo sensor closed")
        time.sleep(0.5)
