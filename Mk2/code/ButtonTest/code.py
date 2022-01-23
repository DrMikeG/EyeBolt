# Write your code here :-)
import time
import board
import digitalio


button10 = digitalio.DigitalInOut(board.GP10)
button10.switch_to_input(pull=digitalio.Pull.UP)
button11 = digitalio.DigitalInOut(board.GP11)
button11.switch_to_input(pull=digitalio.Pull.UP)
button12 = digitalio.DigitalInOut(board.GP12)
button12.switch_to_input(pull=digitalio.Pull.UP)
button13 = digitalio.DigitalInOut(board.GP13)
button13.switch_to_input(pull=digitalio.Pull.UP)

button14 = digitalio.DigitalInOut(board.GP14)
button14.switch_to_input(pull=digitalio.Pull.UP)
button15 = digitalio.DigitalInOut(board.GP15)
button15.switch_to_input(pull=digitalio.Pull.UP)

photoSensorPin2 = digitalio.DigitalInOut(board.GP3)
photoSensorPin2.switch_to_input(pull=digitalio.Pull.UP)


while True:
    if button10.value == False:
        print("You pressed button 10!")
        time.sleep(0.5)
    if button11.value == False:
        print("You pressed button 11!")
        time.sleep(0.5)
    if button12.value == False:
        print("You pressed button 12!")
        time.sleep(0.5)
    if button13.value == False:
        print("You pressed button 13!")
        time.sleep(0.5)
    if button14.value == False:
        print("You pressed button 14!")
        time.sleep(0.5)
    if button15.value == False:
        print("You pressed button 15!")
        time.sleep(0.5)
    if photoSensorPin2.value == True:
        print("Photo sensor closed")
        time.sleep(0.5)