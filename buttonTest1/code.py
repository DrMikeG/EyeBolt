import time
import board
import digitalio

button14 = digitalio.DigitalInOut(board.GP14)
button14.switch_to_input(pull=digitalio.Pull.UP)
button15 = digitalio.DigitalInOut(board.GP15)
button15.switch_to_input(pull=digitalio.Pull.UP)

while True:
    if button14.value == False:
        print("You pressed button 14!")
        time.sleep(0.5)
    if button15.value == False:
        print("You pressed button 15!")
        time.sleep(0.5)