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



def TableHomeCycle():
    print("Table Homing")
    # Move table down until lower limit reached
    while button12.value == True:
        CheckEBreak()
        #Move stepper anticlockwise (down) fast

    # Move table up until upper limit reached
    while button13.value == True:
        CheckEBreak()
        #Move stepper clockwise (up) fast

    # Move slowly down until upper limit switch releases
    while button13.value == True:
        CheckEBreak()
        #Move stepper anticlockwise (down) slow

    # Move slowly up until upper limit switch presses
    while button13.value == False:
        CheckEBreak()
        #Move stepper clockwise (up) slow

    print("Table Home")
    return True


def MovePokerToNearTableEdge():
    CheckEBreak()
    if PokerHasTouched():
        return False # unexpected
    #MovePokerTo()
    for s in range(20):
        CheckEBreak()
        time.sleep(0.1)
    return PokerHasNotTouched()


def FullyRetractPoker():
    CheckEBreak()
    # Change servo to value X
    if PokerHasTouched():
        #MovePokerTo()
        while not PokerIsReset():
            CheckEBreak()
    else:
        #MovePokerTo()
        for s in range(20):
            CheckEBreak()
            time.sleep(0.1)
    CheckEBreak()
    return PokerIsReset()

def PokerHasNotTouched():
    # Sensor open means not touched
    return photoSensorPin2.value == False # False is open

def PokerHasTouched():
    # Sensor closed means not touched
    return photoSensorPin2.value == True # True is closed

def PokerIsReset():
    return PokerHasNotTouched()



def TakeMeasurement():
    print("Measuring")
        
    ok = FullyRetractPoker()
    if not ok : return False
    
    # Send poker towards edge of table
    ok = MovePokerToNearTableEdge()
    if not ok : return False

    ok = FullyRetractPoker()
    if not ok : return False

    # Not touching hopefully
    if PokerHasTouched():
        return False
    
    while PokerHasNotTouched():
        # While not closed
        CheckEBreak()
        # Move poker forward slowly
    # Now closed or at maximum reach
    # Report distance from angle

def PerformScan():
    print("PerformScan")
    # ConfirmFirstMeasureToSideOfTable

    # TurnAndMeasure
    return True


def PokerHomeCycle():
    """Run the homing cycle for the poker

    Fully retract the poker to reset the sensor to open
    Move to near the table edge (doesn't matter on table height)
    Fully retract the poker to reset the sensor to open
    If all that gives the expected sensor readings then poker is home
    """

    print("Poker Homing")

    ok = FullyRetractPoker()
    if not ok : return False
    
    # Send poker towards edge of table
    ok = MovePokerToNearTableEdge()
    if not ok : return False

    ok = FullyRetractPoker()
    if not ok : return False

    print("Poker Home")
    return True


def PrepareThenPerformScan():
    """Runs the pre-scan homing cycles before and after the main scan

    This method homes the poke and the table and checks for errors.
    If there are no errors, it waits for button 14 to be pressed to confirm
    the scan. The scan is performed, then the table and the poker are homed 
    again.
    """
    
    ok = PokerHomeCycle()
    
    if ok:
        ok = TableHomeCycle()
    
    if ok:
        while True:
            CheckEBreak()
            # Wait for initialise, button 14...
            if button14.value == False:
                ok = PerformScan()
            
    if ok:
         ok = PokerHomeCycle()
    if ok:
        ok = TableHomeCycle()

    print("Done!")


while True:
    
    CheckEBreak()

    # Wait for initialise, button 14...
    if button14.value == False:
        PrepareThenPerformScan()
        time.sleep(0.5)

    if button12.value == False:
        print("You pressed button 12!")
        time.sleep(0.5)

    if button13.value == False:
        print("You pressed button 13!")
        time.sleep(0.5)
    
    if photoSensorPin2.value == True:
        print("Photo sensor closed")
        time.sleep(0.5)
