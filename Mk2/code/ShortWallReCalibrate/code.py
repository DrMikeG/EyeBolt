import time
import board
import digitalio
from digitalio import DigitalInOut, Direction, Pull
from adafruit_motor import stepper

# This is designed for the new needle which is 10mm ish longer.
# I'm also increasing the steps per revolution

# Carriage rear stop
button10 = digitalio.DigitalInOut(board.GP10)
button10.switch_to_input(pull=digitalio.Pull.UP)

button11 = digitalio.DigitalInOut(board.GP11)
button11.switch_to_input(pull=digitalio.Pull.UP)
# Lower stop?
button12 = digitalio.DigitalInOut(board.GP12)
button12.switch_to_input(pull=digitalio.Pull.UP)
# Upper stop?
button13 = digitalio.DigitalInOut(board.GP13)
button13.switch_to_input(pull=digitalio.Pull.UP)
# Go?
button14 = digitalio.DigitalInOut(board.GP14)
button14.switch_to_input(pull=digitalio.Pull.UP)
# Panic?
button15 = digitalio.DigitalInOut(board.GP15)
button15.switch_to_input(pull=digitalio.Pull.UP)

photoSensorPin2 = digitalio.DigitalInOut(board.GP3)
photoSensorPin2.switch_to_input(pull=digitalio.Pull.UP)

# Table
stepperAdvanceFromHome = 50 * 100
numberOfRotationsToAttempt = 100

# Poker
maxTrackSteps = 490
# Not sure, but changed down from 100
stepsForPokerToTouchTable = 31

# Stepper motor 1 setup
DELAY = 0.1  # fastest is ~ 0.004, 0.01 is still very smooth, gets steppy after that
STEPS = 200  # this is a full 360º

coils1 = (
    DigitalInOut(board.GP21),  # A1
    DigitalInOut(board.GP20),  # A2
    DigitalInOut(board.GP19),  # B1
    DigitalInOut(board.GP18),  # B2
)
for coil1 in coils1:
    coil1.direction = Direction.OUTPUT

stepper_motor1 = stepper.StepperMotor(
    coils1[0], coils1[1], coils1[2], coils1[3], microsteps=None
)

coils2 = (
    DigitalInOut(board.GP9),  # A1
    DigitalInOut(board.GP8),  # A2
    DigitalInOut(board.GP7),  # B1
    DigitalInOut(board.GP6),  # B2
)
for coil2 in coils2:
    coil2.direction = Direction.OUTPUT

stepper_motor2 = stepper.StepperMotor(
    coils2[0], coils2[1], coils2[2], coils2[3], microsteps=None
)


stepperMgr = {}
stepperMgr['Belt'] = False
stepperMgr['Table'] = False

beltMotor = stepper_motor1
tableMotor = stepper_motor2


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
    stepper_motor1.release()
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
   

def StepTableUpOneStepWithMinDelay():
    AssertTableHasMotorPower() # This might be too slow to check here?
    tableMotor.onestep(direction=stepper.BACKWARD,style=stepper.DOUBLE) # BACKWARDs is table up
    time.sleep(0.002)

def StepTableDownOneStepWithMinDelay():
    AssertTableHasMotorPower() # This might be too slow to check here?
    tableMotor.onestep(direction=stepper.FORWARD,style=stepper.DOUBLE) # FORWARDs is table down
    time.sleep(0.002)

def StepCarriageBackOneStepWithMinDelay():
    AssertBeltHasMotorPower() # This might be too slow to check here?
    beltMotor.onestep(direction=stepper.BACKWARD,style=stepper.DOUBLE) # BACKWARDs is table up
    time.sleep(0.002)

def StepCarriageForwardOneStepWithMinDelay():
    AssertBeltHasMotorPower() # This might be too slow to check here?
    beltMotor.onestep(direction=stepper.FORWARD,style=stepper.DOUBLE) # FORWARDs is table down
    time.sleep(0.002)

def TableIsAtUpperStop():
    return button13.value == False

def TableIsAtLowerStop():
    return button12.value == False

def CarriageAtRearStop():
    #print("Checking CarriageAtRearStop {}".format(button10.value == False))
    return button10.value == False


def TableMoveToUpperLimit():
    # Move table up until upper limit reached
    print("Moving table to upper limit")
    while not TableIsAtUpperStop():
        CheckEBreak()
        #Move stepper clockwise (up) fast
        StepTableUpOneStepWithMinDelay()
    print("Moving table to upper limit - done")

def TableHomeAtPointerTouchHeight():
    print("Table Homing")

    TableTakeStepperPower()

    # Move table up until upper limit reached
    TableMoveToUpperLimit()

    print("Table retract from upper limit")
    # Move slowly down until upper limit switch releases
    while TableIsAtUpperStop():
        CheckEBreak()
        #Move stepper anticlockwise (down) slow
        StepTableDownOneStepWithMinDelay()
        #time.sleep(0.004) # extra delay to slow speed
    print("Table retract from upper limit - done")

    print("Table seek upper limit")
    # Move slowly up until upper limit switch presses
    while not TableIsAtUpperStop():
        CheckEBreak()
        #Move stepper clockwise (up) slow
        StepTableUpOneStepWithMinDelay()
        #time.sleep(0.004) # extra delay to slow speed
    print("Table seek upper limit - done")

    print("Table Homed")
    # disable coils
    TableReleaseStepperPower()
    return True

def MoveTableToScanStartPosition():
    print("MoveTableToScanStartPosition()")
    CheckEBreak()

    TableTakeStepperPower()

    # This assert cannot be the result of mistiming and should not be re-tried
    # This can be wrong if the table has been nudged away from the switch
    TableMoveToUpperLimit()
    
    for _ in range(stepperAdvanceFromHome):
        StepTableDownOneStepWithMinDelay()
        CheckEBreak()
        # This assert cannot be the result of mistiming and should not be re-tried
        if TableIsAtLowerStop():
            print("Table hit lower stop!?")
            Panic()
            return False

    TableReleaseStepperPower()   

    print("MoveTableToScanStartPosition() - done")
    return True

def MovePokerToNearTableEdge():
    # Move poker to be near the edge of the table, but not touching
    print("MovePokerToNearTableEdge")
    #MoveServo(servoPositionB)

def MovePokerToAdvancePositionOverTable():
    # Move poker to beyond the table edge, but before any expected reading
    #MoveServo(servoPositionE)
    time.sleep(0.5)

def FullyRetractPokerAndCheckSensor():
    CheckEBreak()
    print("FullyRetractPoker")
    # Change servo to value X
    pokerCanTellUsWhenWeAreFullyRetracted = PokerHasTouched()

    #print("moving servo to "+str(servoPositionA)+" from "+str(servoCurrentTarget))
    #MoveServo(servoPositionA)
    
    if pokerCanTellUsWhenWeAreFullyRetracted:
        retryCount = 0
        while not PokerIsReset() and retryCount < 5:
            #MoveServo(servoPositionA)
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
            #MoveServo(servoPositionB)
            #MoveServo(servoPositionA)
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

    if not CarriageAtRearStop():
        print("** ERROR ** TakeMeasurement() Carriage not are rear stop")
        return False, False, 0

    if PokerHasTouched():
        print("** ERROR ** TakeMeasurement() PokerHasTouched cannot measure")
        return False, False, 0

    BeltTakeStepperPower()

    # Move closer to save time
    # Nothing should register before servoPositionE
    # Not yet supported
    #if (allowFastforward):
    #    MovePokerToAdvancePositionOverTable()

    if PokerHasTouched():
        print("** ERROR ** TakeMeasurement() When staging poker, touch sensor was triggered")
        return False, False, 0

    # ignore limit for now
    #print("Measuring - starting")
    measureSteps =0 
    hasTouched = False

    while not hasTouched:
        StepCarriageForwardOneStepWithMinDelay()
        CheckEBreak()
        time.sleep(0.001)
        measureSteps = measureSteps + 1
        hasTouched = PokerHasTouched()
        # Step before end of track
        if measureSteps >= maxTrackSteps:
            break

    #print("Measuring - done")

    BeltReleaseStepperPower()

    # PokerHome Takes Power for itself
    ok = PokerHome()

    return ok, hasTouched, measureSteps


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

    if (abs(measureAngle-stepsForPokerToTouchTable) <= 5):
        ok = True
        print("ConfirmFirstMeasureToSideOfTable() - done")
    else:
        ok = False
        print("ConfirmFirstMeasureToSideOfTable() - failed")
    
    return ok


def StepNextChunkAndTakeMeasurement(stepsToChunk,chunkNumber,rotationNumber):
    # Advance table down
    hasTouched = False

    TableTakeStepperPower()

    for _ in range(stepsToChunk):
        CheckEBreak()
        StepTableDownOneStepWithMinDelay()

        if TableIsAtLowerStop():
            print("Lower limit reached")
            Panic()
            return False, hasTouched
    
    # disable coils
    TableReleaseStepperPower()

    # Take measurement
    ok, hasTouched, measuredSteps = TakeMeasurement()
    if not ok:
        print("Error in TakeMeasurement()")
        return False, hasTouched
    print("%i:%i touched %s steps %s" % (rotationNumber,chunkNumber,str(hasTouched),str(measuredSteps)))
    
    return True, hasTouched

def MakeOneFullRevolvePrintingSteps(rotationNumberForLeapStep):
    print("Revolution "+str(rotationNumberForLeapStep))
    
    hasTouchedInThisRevolution = False

    # A series of chunks makes up a revolution (chunks are not quite uniform divisions)
    chunk = 0
    # 97 lots of 13 steps
    #  5 lots of 12 steps
    #  1 lot of 27 + (1 if rev % 3 = 0) which adds the .3
    for stepCount in range(97):
        ok, hasTouched = StepNextChunkAndTakeMeasurement(13,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        chunk = chunk + 1
        if not ok : return False

    for stepCount in range(5):
        ok, hasTouched = StepNextChunkAndTakeMeasurement(12,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        chunk = chunk + 1
        if not ok : return False
            
    if rotationNumberForLeapStep % 3 == 0 :
        ok, hasTouched = StepNextChunkAndTakeMeasurement(12,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        chunk = chunk + 1
        if not ok : return False
    else:
        # Lead step for sync
        ok, hasTouched = StepNextChunkAndTakeMeasurement(13,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        chunk = chunk + 1
        if not ok : return False
    
    return True, hasTouchedInThisRevolution

def PerformScan():
    print("PerformScan")

    rotationCount = 0
    success = True

    while rotationCount < numberOfRotationsToAttempt:
        ok, hasTouchedInThisRevolution = MakeOneFullRevolvePrintingSteps(rotationCount)
        # If there was an error then stop
        if not ok:
            success = False
            break
        # If there were no measurements for a whole revolution then stop
        if not hasTouchedInThisRevolution:
            print("** INFO ** Did not touch object during previous revolution so stopping scan")    
            break
        rotationCount = rotationCount + 1

    print("PerformScan - done (revolutions %u)" %(rotationCount))
    return success

def AssertBeltHasMotorPower():
    assert stepperMgr['Belt'] ==True
def AssertTableHasMotorPower():
    assert stepperMgr['Table'] ==True
def AssertNeitherMotorHasMotorPower():
    assert stepperMgr['Belt'] ==False
    assert stepperMgr['Table'] ==False

# You can only take control of the power if power is not current assigned to either motor
# else we are in an error condition
def BeltTakeStepperPower():
    AssertNeitherMotorHasMotorPower()
    stepperMgr['Belt'] = True

def TableTakeStepperPower():
    AssertNeitherMotorHasMotorPower()
    stepperMgr['Table'] = True

# You can only release the power if it is currently assigned to the motor that is releasing it
# else we are in an error condition
def BeltReleaseStepperPower():
    AssertBeltHasMotorPower()
    stepperMgr['Belt'] =False
    # disable coils
    beltMotor.release()
    #print("Belt motor released")

def TableReleaseStepperPower():
    AssertTableHasMotorPower()
    stepperMgr['Table'] =False
    tableMotor.release()
    #print("Table motor released")

def PokerHome():
    """Run the homing cycle for the poker
    """

    #print("Poker Homing")

    BeltTakeStepperPower()

    # Go Home
    while not CarriageAtRearStop():
        StepCarriageBackOneStepWithMinDelay()
        CheckEBreak()
    
    # Gently leave Home
    while CarriageAtRearStop():
        StepCarriageForwardOneStepWithMinDelay()
        time.sleep(0.01)
        CheckEBreak()
    # Go Home
    while not CarriageAtRearStop():
        StepCarriageBackOneStepWithMinDelay()
        time.sleep(0.01)
        CheckEBreak()
    
    BeltReleaseStepperPower()

    # Confirm poker sensor is reset
    assert PokerIsReset()

    #print("Fully retract and confirm sensor reset - done")

    return True


def PrepareThenPerformScan():
    """Runs the pre-scan homing cycles before and after the main scan

    This method homes the poke and the table and checks for errors.
    If there are no errors, it waits for button 14 to be pressed to confirm
    the scan. The scan is performed, then the table and the poker are homed
    again.
    """

    ok = True

    if ok:
        ok = PokerHome()

    if ok:
        ok = TableHomeAtPointerTouchHeight()
    
    if ok: 
        ok = ConfirmFirstMeasureToSideOfTable()

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
         ok = PokerHome()
    
    print("Done!")

CheckEBreak()

while True:

    # Wait for initialise, button 14...
    if button14.value == False:
        PrepareThenPerformScan()
        break

    print("Button15(PANIC) \t\t{0}".format(        ["Not pressed", "Pressed"][button15.value == False]))
    print("Button14(Go) \t\t{0}".format(        ["Not pressed", "Pressed"][button14.value == False]))
    print("Button13(UpperLimit) \t{0}".format(["Not pressed", "Pressed"][button13.value == False]))
    print("Button12(LowerLimit) \t{0}".format(["Not pressed", "Pressed"][button12.value == False]))
    print("Button11(.) \t\t{0}".format(["Not pressed", "Pressed"][button11.value == False]))
    print("Button10(CarriageLimit) \t{0}".format(["Not pressed", "Pressed"][button10.value == False]))
    print("PhotoSensor \t\t{0}".format(["Open", "Closed"][photoSensorPin2.value == True]))
    
    print("Waiting...")
    time.sleep(0.1)