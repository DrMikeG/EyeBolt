# Integrating servo home button

import time
import board
import digitalio
import pwmio
from digitalio import DigitalInOut, Direction, Pull
from adafruit_motor import stepper, servo


# Servo control
servoPositionA = 155 # All the way back from the table
servoPositionB = 139 # Near edge of table
servoPositionC = 135 # Touching edge of table
servoPositionD = 70 # At full reach across the table
servoPositionE = 115 # skip measurements before this (nothing outside this)

servoCurrentTarget = servoPositionB

maxTableRevolutions = 75
stepperAdvanceFromHome = 40 * 100

# create a PWMOut object on Pin A2.
pwm = pwmio.PWMOut(board.GP2, duty_cycle=2 ** 15, frequency=50)
my_servo = servo.Servo(pwm)

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

# Lower table stop
button12 = digitalio.DigitalInOut(board.GP12)
button12.switch_to_input(pull=digitalio.Pull.UP)

# Upper table stop
button13 = digitalio.DigitalInOut(board.GP13)
button13.switch_to_input(pull=digitalio.Pull.UP)

# Servo home table stop
button22 = digitalio.DigitalInOut(board.GP22)
button22.switch_to_input(pull=digitalio.Pull.UP)



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

def Panic():
    # Park Servo
    print("PANIC")
    stepper_motor.release()
    # Don't use MoveServo as that calls CheckEBreak
    my_servo.angle = servoPositionB
    while True:
        if button14.value == False:
            print("OK button releases panic")
            break
        time.sleep(0.1)

def CheckEBreak():
    # Button 15 is the EBreak - if this is pressed then:
    # Send Servo at safe value
    # Wait for OK
    if button15.value == False:
        Panic()

def CheckServoPositionIsInRangeAndPanic(newTargetAngle):
    # This assert cannot be the result of mistiming and should not be re-tried
    if newTargetAngle > servoPositionA:
        print("** ERROR ** ServoPosition %s is beyond full retract of arm",str(newTargetAngle))    
        Panic()
    # This assert cannot be the result of mistiming and should not be re-tried
    if newTargetAngle < servoPositionD:
        print("** ERROR ** ServoPosition %s is beyond full extent of arm",str(newTargetAngle))
        Panic()        

def MoveServo(newTargetAngle):
    global servoCurrentTarget
    CheckEBreak()
    CheckServoPositionIsInRangeAndPanic(newTargetAngle)

    servoCurrentTarget = newTargetAngle
    my_servo.angle = servoCurrentTarget
    for _ in range(10):
        CheckEBreak()
        time.sleep(0.025)

def ServoHomeSwitchPressed():
    CheckEBreak()
    # Return True if the home switch has been pressed.
    return button22.value == False

def ServoHomeCycle():
    nonHomeValue = 115
    homeValue = 140

    print("ServoHomeCycle - start")
    #Servo home is expected to be slightly more than 140
    MoveServo(nonHomeValue)
    print("ServoHomeCycle - start")
    time.sleep(1)
    if ServoHomeSwitchPressed():
        print("ServoHomeCycle failed - did not expect home switch pressed with current servo angle")    
        return False

    # Decrease servo value    
    foundHomingValue = False
    while not foundHomingValue:
        homeValue = homeValue + 1
        print("Testing possible home value {}".format(homeValue))
        MoveServo(homeValue)        
        time.sleep(0.75)
        if ServoHomeSwitchPressed():
            print("Moving to home value {} triggered the home switch".format(homeValue))
            foundHomingValue = True
        print("Moving back away from home switch {}".format(nonHomeValue))
        MoveServo(nonHomeValue)
        time.sleep(0.75)

    if not foundHomingValue:
        print("ServoHomeCycle failed - did not find value at which servo home switch was pressed")
        return False
    else:
        
        print("Confirming home location triggers switch")
        MoveServo(homeValue)
        time.sleep(0.75)
        if not ServoHomeSwitchPressed():
            print("Expected home home value {} to trigger the home switch".format(homeValue))
            return False
        else:
            print("Confirmed")
            print("Moving back away from home switch {}".format(nonHomeValue))
            MoveServo(nonHomeValue)
            time.sleep(0.75)
            
        print("Confirming home location -1 does not trigger switch")
        MoveServo(homeValue-1)
        time.sleep(0.75)
        if ServoHomeSwitchPressed():
            print("Did not expected home home value -1 {} to trigger the home switch".format(homeValue-1))
            return False
        MoveServo(nonHomeValue)
        time.sleep(0.75)
    
    print("ServoHomeCycle - end")


def StepTableUpOneStepWithDelay():
    stepper_motor.onestep(direction=stepper.BACKWARD,style=stepper.DOUBLE) # BACKWARDs is table up
    time.sleep(0.002)

def StepTableDownOneStepWithDelay():
    stepper_motor.onestep(direction=stepper.FORWARD,style=stepper.DOUBLE) # FORWARDs is table down
    time.sleep(0.002)

def TableIsAtUpperStop():
    return button13.value == False

def TableIsAtLowerStop():
    return button12.value == False

def TableHomeCycle():
    print("Table Homing")

    # Move table up until upper limit reached
    print("Moving table to upper limit")
    while not TableIsAtUpperStop():
        CheckEBreak()
        #Move stepper clockwise (up) fast
        StepTableUpOneStepWithDelay()
    print("Moving table to upper limit - done")

    print("Table retract from upper limit")
    # Move slowly down until upper limit switch releases
    while TableIsAtUpperStop():
        CheckEBreak()
        #Move stepper anticlockwise (down) slow
        StepTableDownOneStepWithDelay()
        #time.sleep(0.004) # extra delay to slow speed
    print("Table retract from upper limit - done")

    print("Table seek upper limit")
    # Move slowly up until upper limit switch presses
    while not TableIsAtUpperStop():
        CheckEBreak()
        #Move stepper clockwise (up) slow
        StepTableUpOneStepWithDelay()
        #time.sleep(0.004) # extra delay to slow speed
    print("Table seek upper limit - done")

    print("Table Homed")
    # disable coils
    stepper_motor.release()

    return True

def MoveTableToScanStartPosition():
    print("MoveTableToScanStartPosition()")
    CheckEBreak()

    # This assert cannot be the result of mistiming and should not be re-tried
    # This can be wrong if the table has been nudged away from the switch
    if not TableIsAtUpperStop():
        print("Table not at upper stop?")
        return False
    
    for _ in range(stepperAdvanceFromHome):
        StepTableDownOneStepWithDelay()
        CheckEBreak()
        # This assert cannot be the result of mistiming and should not be re-tried
        if TableIsAtLowerStop():
            print("Table hit lower stop!?")
            Panic()
            return False

    # disable coils
    stepper_motor.release()
    print("MoveTableToScanStartPosition() - done")
    return True

def MovePokerToNearTableEdge():
    # Move poker to be near the edge of the table, but not touching
    #print("MovePokerToNearTableEdge")
    MoveServo(servoPositionB)

def MovePokerToAdvancePositionOverTable():
    # Move poker to beyond the table edge, but before any expected reading
    MoveServo(servoPositionE)
    time.sleep(0.5)

def FullyRetractPokerAndCheckSensor():
    CheckEBreak()
    print("FullyRetractPoker")
    # Change servo to value X
    pokerCanTellUsWhenWeAreFullyRetracted = PokerHasTouched()

    print("moving servo to "+str(servoPositionA)+" from "+str(servoCurrentTarget))
    MoveServo(servoPositionA)
    
    if pokerCanTellUsWhenWeAreFullyRetracted:
        retryCount = 0
        while not PokerIsReset() and retryCount < 5:
            MoveServo(servoPositionA)
            print("Retry...")
            retryCount = retryCount + 1
                
        if not PokerIsReset():
            print("** Error waiting for Poker to fully retract from PokerHasTouched position ")
            Panic()
    else:
        # Wait half a second for servo to more
        time.sleep(0.50)
        if not PokerIsReset():
            print("Poker did not reset within 0.5 - second chance...")
            MoveServo(servoPositionB)
            MoveServo(servoPositionA)
            time.sleep(0.50)

    return PokerIsReset()

def PokerHasNotTouched():
    # Sensor open means not touched
    return photoSensorPin2.value == False # False is open

def PokerHasTouched():
    # Sensor closed means not touched
    return photoSensorPin2.value == True # True is closed

def PokerIsReset():
    return PokerHasNotTouched()



def TakeMeasurement(allowFastforward = True):
    global servoCurrentTarget

    #print("Measuring")

    ok = FullyRetractPokerAndCheckSensor()
    if not ok : 
        print("FullyRetractPokerAndCheckSensor at start of TakeMeasurement() failed - second chance...")
        ok = FullyRetractPokerAndCheckSensor()
        if not ok : 
            print("** ERROR ** TakeMeasurement() FullyRetractPoker() did not succeed in expected time")
            return False, False, 0

    # Move closer to save time
    # Nothing should register before servoPositionE
    if (allowFastforward):
        MovePokerToAdvancePositionOverTable()

    if PokerHasTouched():
        print("** Warning** When staging poker, touch sensor was triggered - attempting another reset...")
        ok = FullyRetractPokerAndCheckSensor()
        if not ok or PokerHasTouched():
            print("** ERROR ** TakeMeasurement() When staging poker, touch sensor was triggered - recovery failed")
            return False, False, 0

    # ignore limit for now
    # print("Measuring - starting")
    measureAngle = servoCurrentTarget
    hasTouched = False

    while not hasTouched:
        measureAngle = measureAngle - 1
        MoveServo(measureAngle)
        hasTouched = PokerHasTouched()
        if measureAngle == servoPositionD:
            break

    #print("Measuring - done")

    ok = FullyRetractPokerAndCheckSensor()
    if not ok : 
        print("** ERROR ** TakeMeasurement() FullyRetractPoker() did not succeed after measurement on first attempt - 2nd chance")    
        ok = FullyRetractPokerAndCheckSensor()
        if not ok : 
            print("** ERROR ** TakeMeasurement() FullyRetractPoker() did not succeed")    
            return False, False, 0

    return ok, hasTouched, measureAngle


def ConfirmFirstMeasureToSideOfTable():
    print("ConfirmFirstMeasureToSideOfTable()")
    allowFastFwd = False # table will be in the way, so cannot sneak in to reduced radius of measurement
    ok, hasTouched, measureAngle = TakeMeasurement(allowFastFwd)

    if not ok:
         print("Error in TakeMeasurement()")
         return False

    if not hasTouched:
         print("Error Confirming first measurement to side of table - did not touch")
         return False

    print("Table measured at :"+str(measureAngle))

    if (abs(measureAngle - servoPositionC) < 3):
        ok = True
        print("ConfirmFirstMeasureToSideOfTable() - done")
    else:
        ok = False
        print("ConfirmFirstMeasureToSideOfTable() - failed")
    
    return ok

def Nudge():
    print("Nudge?")
    time.sleep(1.0)
    while True:
        CheckEBreak()
        if button14.value == False:
            time.sleep(0.2)
            while button14.value == False:
                StepTableDownOneStepWithDelay()
                time.sleep(0.05)
            return

def RotateDownToFirstTouch():
    # Just move roughly doing measurements until we find the stick
    for stepCount in range(100):
        for _ in range(26):
            CheckEBreak()
            StepTableDownOneStepWithDelay()
        
        ok, hasTouched, measureAngle = TakeMeasurement()
        if not ok:
            print("Error in TakeMeasurement()")
            return False
        if hasTouched and measureAngle < 133:
            return True
    
    return False


def StepNextChunkAndTakeMeasurement(stepsToChunk,chunkNumber,rotationNumber):
    # Advance table down
    hasTouched = False

    # enable coils

    for _ in range(stepsToChunk):
        CheckEBreak()
        StepTableDownOneStepWithDelay()

        if TableIsAtLowerStop():
            print("Lower limit reached")
            Panic()
            return False, hasTouched
    
    # disable coils
    stepper_motor.release()

    # Take measurement
    ok, hasTouched, measureAngle = TakeMeasurement()
    if not ok:
        print("Error in TakeMeasurement()")
        return False, hasTouched
    print("%i:%i touched %s angle %s" % (rotationNumber,chunkNumber,str(hasTouched),str(measureAngle)))
    
    return True, hasTouched

def MakeOneFullRevolvePrintingAngles(rotationNumberForLeapStep):
    print("Revolution "+str(rotationNumberForLeapStep))
    
    hasTouchedInThisRevolution = False

    # A series of chunks makes up a revolution (chunks are not quite uniform divisions)
    chunk = 0
    # 44 lots of 26 steps
    #  6 lots of 27 steps
    #  1 lot of 27 + (1 if rev % 3 = 0) which adds the .3
    for stepCount in range(44):
        ok, hasTouched = StepNextChunkAndTakeMeasurement(26,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        chunk = chunk + 1
        if not ok : return False

    for stepCount in range(6):
        ok, hasTouched = StepNextChunkAndTakeMeasurement(27,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        chunk = chunk + 1
        if not ok : return False
            
    if rotationNumberForLeapStep % 3 == 0 :
        ok, hasTouched = StepNextChunkAndTakeMeasurement(27,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        chunk = chunk + 1
        if not ok : return False
    else:
        # Lead step for sync
        ok, hasTouched = StepNextChunkAndTakeMeasurement(28,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        chunk = chunk + 1
        if not ok : return False
    
    return True, hasTouchedInThisRevolution

def PerformScan():
    print("PerformScan")

    rotationCount = 0
    success = True

    while rotationCount < maxTableRevolutions:
        ok, hasTouchedInThisRevolution = MakeOneFullRevolvePrintingAngles(rotationCount)
        # If there was an error then stop
        if not ok:
            success = False
            break
        # If there were no measurements for a whole revolution then stop
        if not hasTouchedInThisRevolution:
            print("** INFO ** Did not touch object during previous revolution so stopping scan")    
            break
        rotationCount = rotationCount + 1

    print("PerformScan - done (revolutions %u",rotationCount)
    return True


def PokerHomeCycle():
    """Run the homing cycle for the poker

    Fully retract the poker to reset the sensor to open
    Move to near the table edge (doesn't matter on table height)
    Fully retract the poker to reset the sensor to open
    If all that gives the expected sensor readings then poker is home
    """

    print("Poker Homing")

    ok = FullyRetractPokerAndCheckSensor()
    if not ok : 
        print("** ERROR ** PokerHomeCycle() FullyRetractPoker() did not succeed")    
        return False
    time.sleep(1.0)
    print("Fully retract and confirm sensor reset - done")

    MovePokerToNearTableEdge()

    if PokerHasTouched():
        print("** ERROR ** PokerHomeCycle() When staging poker, touch sensor was triggered")    
        return False
    time.sleep(1.0)
    print("Stage Poker and confirm sensor available - done")

    ok = FullyRetractPokerAndCheckSensor()
    if not ok : 
        print("** ERROR ** PokerHomeCycle() FullyRetractPoker() did not succeed")    
        return False
    time.sleep(1.0)
    print("Fully retract poker ready for table movement - done")

    print("Poker Homed")
    return True


def PrepareThenPerformScan():
    """Runs the pre-scan homing cycles before and after the main scan

    This method homes the poke and the table and checks for errors.
    If there are no errors, it waits for button 14 to be pressed to confirm
    the scan. The scan is performed, then the table and the poker are homed
    again.
    """

    ServoHomeCycle()
    return True

    ok = FullyRetractPokerAndCheckSensor()
    
    if ok:
        ok = TableHomeCycle()

    if ok:
        ok = PokerHomeCycle()

    if ok:
        ok = ConfirmFirstMeasureToSideOfTable()
    
    if ok:
        return True

    if ok:
        ok = MoveTableToScanStartPosition()

    if ok:
        print("Press confirm to begin scan?")
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


for _ in range(400):
    StepTableDownOneStepWithDelay()

    
