import time
import board
import digitalio
from digitalio import DigitalInOut, Direction, Pull
from adafruit_motor import stepper
import analogio

# Thumbstick
thumbSwitch = digitalio.DigitalInOut(board.GP22)
thumbSwitch.switch_to_input(pull=digitalio.Pull.UP)

thumbLeftRight = analogio.AnalogIn(board.GP26)
thumbUpDown = analogio.AnalogIn(board.GP27)


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

tableStepsDownToStartPosition = 20 * 100
numberOfRotationsToAttempt = 100

maxTrackSteps = 460
midTrackSteps = 235

stepsBetweenMeasures = 4
nMeasurementsPerSweep = 115

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

beltMotor = stepper_motor2
tableMotor = stepper_motor1

## LED ##########################

led = DigitalInOut(board.LED)
led.direction = Direction.OUTPUT
led.value = True

def blink(times):
    for _ in range(times):
        led.value = False
        time.sleep(0.1)
        led.value = True
        time.sleep(0.1)

## Sensor inputs ##########################

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

def isLeft():
    # 0 or 65535
    return thumbLeftRight.value < 16383

def isRight():
    # 0 or 65535
    return thumbLeftRight.value > 49149

def isUp():
    # 0 or 65535
    return thumbUpDown.value > 49149
        
def isDown():
    # 0 or 65535
    return thumbUpDown.value < 16383

def TableIsAtUpperStop():
    return button13.value == False

def TableIsAtLowerStop():
    return button12.value == False

def CarriageAtRearStop():
    #print("Checking CarriageAtRearStop {}".format(button10.value == False))
    return button10.value == False

def BeamIsNotBroken():
    # Sensor open means not touched
    return photoSensorPin2.value == False # False is open

def BeamIsBroken():
    # Sensor closed means not touched
    return photoSensorPin2.value == True # True is closed


## Motor Movements (RP = require power) ###################
# You have to take correct motor out of standby before calling these methods

def rpStepTableUpOneStepWithMinDelay():
    AssertTableHasMotorPower() # This might be too slow to check here?
    tableMotor.onestep(direction=stepper.FORWARD,style=stepper.DOUBLE) # BACKWARDs is table up
    time.sleep(0.002)

def rpStepTableDownOneStepWithMinDelay():
    AssertTableHasMotorPower() # This might be too slow to check here?
    tableMotor.onestep(direction=stepper.BACKWARD,style=stepper.DOUBLE) # FORWARDs is table down
    time.sleep(0.002)

def rpStepCarriageBackOneStepWithMinDelay():
    AssertBeltHasMotorPower() # This might be too slow to check here?
    beltMotor.onestep(direction=stepper.BACKWARD,style=stepper.DOUBLE) # BACKWARDs is table up
    time.sleep(0.002)

def rpStepCarriageForwardOneStepWithMinDelay():
    AssertBeltHasMotorPower() # This might be too slow to check here?
    beltMotor.onestep(direction=stepper.FORWARD,style=stepper.DOUBLE) # FORWARDs is table down
    time.sleep(0.002)

def rpTableMoveToUpperLimit():
    # Move table up until upper limit reached
    print("Moving table to upper limit")
    while not TableIsAtUpperStop():
        CheckEBreak()
        #Move stepper clockwise (up) fast
        rpStepTableUpOneStepWithMinDelay()
    print("Moving table to upper limit - done")

## Power control and test ###############################
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

## Move one or more elements ############################

# Assign motor power and move one or more thing in a set way
# Release power at the end

def MoveTableToPreScanPosition():
    print("MoveTableToPreScanPosition()")
    CheckEBreak()
    TableTakeStepperPower()

    # This assert cannot be the result of mistiming and should not be re-tried
    # This can be wrong if the table has been nudged away from the switch
    rpTableMoveToUpperLimit()

    TableReleaseStepperPower()

    print("MoveTableToPreScanPosition() - done")
    return True

def MoveGantryAndTableToScanStartPosition():

    print("MoveGantryAndTableToScanStartPosition()")
    CheckEBreak()

    TableTakeStepperPower()

    # This assert cannot be the result of mistiming and should not be re-tried
    # This can be wrong if the table has been nudged away from the switch
    rpTableMoveToUpperLimit()

    for _ in range(tableStepsDownToStartPosition):
        rpStepTableDownOneStepWithMinDelay()
        CheckEBreak()
        # This assert cannot be the result of mistiming and should not be re-tried
        if TableIsAtLowerStop():
            print("Table hit lower stop!?")
            Panic()
            return False

    TableReleaseStepperPower()

    # Home gantry (should already by home, but...)
    LaserGantryHome(True)

    # Move gantry to midpoint
    BeltTakeStepperPower()
    for _ in range(midTrackSteps):
        rpStepCarriageForwardOneStepWithMinDelay()
        CheckEBreak()
        time.sleep(0.001)
    BeltReleaseStepperPower()

    print("MoveGantryAndTableToScanStartPosition() - done")
    print("Laser should be glancing over top of table")
    return True

def LaserGantryHome(fast):

    if not fast:
        print("Homing Laser gantry")

    BeltTakeStepperPower()

    # Go Home
    while not CarriageAtRearStop():
        rpStepCarriageBackOneStepWithMinDelay()
        CheckEBreak()

    if not fast:
        # Gently leave Home
        while CarriageAtRearStop():
            rpStepCarriageForwardOneStepWithMinDelay()
            time.sleep(0.01)
            CheckEBreak()

        # Go Home
        while not CarriageAtRearStop():
            rpStepCarriageBackOneStepWithMinDelay()
            CheckEBreak()

    BeltReleaseStepperPower()

    if BeamIsBroken():
        time.sleep(0.025)

    if BeamIsBroken():
        print("Stabilising pause...")
        time.sleep(1.0)

    if BeamIsBroken():
        print(" ** Realignement needed! **")
        print("Press button to continue scan")
        while True:
            CheckEBreak()
            # Wait for initialise, button 14...
            if button14.value == False:
                break
            if BeamIsNotBroken():
                break

    if not fast:
        print("Fully retracted and confirmed laser received - done")

    return True

def LaserGantryTestSweep():
    print("Test sweep of Laser gantry")

    assert BeamIsNotBroken()

    BeltTakeStepperPower()

    # Go Home
    while not CarriageAtRearStop():
        rpStepCarriageBackOneStepWithMinDelay()
        CheckEBreak()


    beamBrokenAtSomePoint = False

    # Move gantry to limit
    for _ in range(maxTrackSteps):
        rpStepCarriageForwardOneStepWithMinDelay()
        if BeamIsBroken:
            beamBrokenAtSomePoint = True
        CheckEBreak()
        time.sleep(0.001)

    # Go Home
    while not CarriageAtRearStop():
        rpStepCarriageBackOneStepWithMinDelay()
        CheckEBreak()

    BeltReleaseStepperPower()

    assert BeamIsNotBroken()
    print("Test sweep of Laser gantry - done")

    return beamBrokenAtSomePoint


def TakeMeasurements():

    #print("Measuring sweep")

    #if not CarriageAtRearStop():
    #    print("** ERROR ** TakeMeasurements() Carriage not are rear stop")
    #    return False, False, 0

    #if BeamIsBroken():
    #    print("** ERROR ** TakeMeasurements() Beam is broken at start of sweep cannot measure")
    #    return False, False, 0

    BeltTakeStepperPower()

    #nMeasurementsPerSweep = 100
    beamMeasurements = []

    beamBroken = False

    for stepsInSweep in range(nMeasurementsPerSweep):
        if BeamIsBroken():
            beamMeasurements.append(1)
            beamBroken = True
        else:
            beamMeasurements.append(0)

        for _ in range(stepsBetweenMeasures):
            rpStepCarriageForwardOneStepWithMinDelay()
            CheckEBreak()
            time.sleep(0.001)

    #print("Measuring sweep - done")

    BeltReleaseStepperPower()

    # PokerHome Takes Power for itself
    ok = LaserGantryHome(True)

    return ok, beamBroken, beamMeasurements


def StepNextChunkAndTakeMeasurement(stepsToChunk,chunkNumber,rotationNumber):

    beamBroken = False

    TableTakeStepperPower()

    for _ in range(stepsToChunk):
        CheckEBreak()
        rpStepTableDownOneStepWithMinDelay()

        if TableIsAtLowerStop():
            print("Lower limit reached")
            Panic()
            return False, beamBroken

    # disable coils
    TableReleaseStepperPower()

    # Perform measurement sweep
    ok, beamBroken, beamMeasurements = TakeMeasurements()
    if not ok:
        print("Error in TakeMeasurements()")
        return False, beamBroken

    # Print measurements
    print("%i:%i beam Broken %s steps %s" % (rotationNumber,chunkNumber,str(beamBroken),str(beamMeasurements)))

    return True, beamBroken

def MakeOneFullRevolvePrintingSteps(rotationNumberForLeapStep):
    print("Revolution "+str(rotationNumberForLeapStep))

    hasTouchedInThisRevolution = False

    # A series of chunks makes up a revolution (chunks are not quite uniform divisions)
    chunk = 0
#| 71 | 12   | 852   |   |
#| 36  | 13   | 458     |   |
#| 1  | 13.3 | 13.3   |   |

    # 71 lots of 12 steps
    # 36 lots of 13 steps
    #  1 lot of 13 + (1 if rev % 3 = 0) which adds the .3

    # I want to distribute the 12s and 13s -
    # so do 2 12 then 1 13 and repeat 35 times
    # that leaves 1 12 and 1 13

    for _ in range(35):
        # 12
        ok, hasTouched = StepNextChunkAndTakeMeasurement(12,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        else:
            print("** INFO ** Object did not break beam during previous revolution so stopping scan")
            return True, False
        chunk = chunk + 1
        if not ok : return False
        # 12
        ok, hasTouched = StepNextChunkAndTakeMeasurement(12,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        else:
            print("** INFO ** Object did not break beam during previous revolution so stopping scan")
            return True, False
        chunk = chunk + 1
        if not ok : return False
        # 13
        ok, hasTouched = StepNextChunkAndTakeMeasurement(13,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        else:
            print("** INFO ** Object did not break beam during previous revolution so stopping scan")
            return True, False
        chunk = chunk + 1
        if not ok : return False

    # 12
    ok, hasTouched = StepNextChunkAndTakeMeasurement(12,chunk,rotationNumberForLeapStep)
    if hasTouched:
        hasTouchedInThisRevolution = True
    else:
        print("** INFO ** Object did not break beam during previous revolution so stopping scan")
        return True, False
    chunk = chunk + 1
    if not ok : return False

    # 13
    ok, hasTouched = StepNextChunkAndTakeMeasurement(13,chunk,rotationNumberForLeapStep)
    if hasTouched:
        hasTouchedInThisRevolution = True
    else:
        print("** INFO ** Object did not break beam during previous revolution so stopping scan")
        return True, False
    chunk = chunk + 1
    if not ok : return False

    # 13.3
    if rotationNumberForLeapStep % 3 == 0 :
        ok, hasTouched = StepNextChunkAndTakeMeasurement(13,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        else:
            print("** INFO ** Object did not break beam during previous revolution so stopping scan")
            return True, False
        chunk = chunk + 1
        if not ok : return False
    else:
        # Lead step for sync
        ok, hasTouched = StepNextChunkAndTakeMeasurement(14,chunk,rotationNumberForLeapStep)
        if hasTouched:
            hasTouchedInThisRevolution = True
        else:
            print("** INFO ** Object did not break beam during previous revolution so stopping scan")
            return True, False
        chunk = chunk + 1
        if not ok : return False

    return True, hasTouchedInThisRevolution

def PerformScan():
    print("PerformScan")

    LaserGantryHome(True)

    rotationCount = 0
    success = True

    while rotationCount < numberOfRotationsToAttempt:
        ok, beamWasBrokenOnThisRevolution = MakeOneFullRevolvePrintingSteps(rotationCount)
        # If there was an error then stop
        if not ok:
            success = False
            break
        # If there were no measurements for a whole revolution then stop
        if not beamWasBrokenOnThisRevolution:
            print("** INFO ** Object did not break beam during previous revolution so stopping scan")
            break
        rotationCount = rotationCount + 1

    print("PerformScan - done (revolutions %u)" %(rotationCount))
    return success

def PrepareThenPerformScan():


    ok = True

    if ok:
        ok = LaserGantryHome(False)

    if ok:
        ok = MoveTableToPreScanPosition()

    if ok:
        ok = LaserGantryTestSweep()

    if ok:
        ok = MoveGantryAndTableToScanStartPosition()

    if ok:
        print("Press confirm to begin scan?")
        while True:
            CheckEBreak()
            # Wait for initialise, button 14...
            if button14.value == False:
                ok = PerformScan()

    if ok:
         ok = LaserGantryHome(False)

    print("Done!")

CheckEBreak()

while True:

    # Wait for initialise, button 14...
    if button14.value == False:
        #PrepareThenPerformScan()
        break

    #print("Button15(PANIC) \t\t{0}".format(        ["Not pressed", "Pressed"][button15.value == False]))
    #print("Button14(Go) \t\t{0}".format(        ["Not pressed", "Pressed"][button14.value == False]))
    #print("Button13(UpperLimit) \t{0}".format(["Not pressed", "Pressed"][button13.value == False]))
    #print("Button12(LowerLimit) \t{0}".format(["Not pressed", "Pressed"][button12.value == False]))
    #print("Button11(.) \t\t{0}".format(["Not pressed", "Pressed"][button11.value == False]))
    #print("Button10(CarriageLimit) \t{0}".format(["Not pressed", "Pressed"][button10.value == False]))
    #print("PhotoSensor \t\t{0}".format(["Open", "Closed"][photoSensorPin2.value == True]))

    if thumbSwitch.value == False:
        print("Thumb switch is pressed")
    if isLeft():
        print("Left")
    if isRight():
        print("Right")
    if isUp():
        print("Up")
        print("Down")
        TableTakeStepperPower()
        for _ in range(100):
            CheckEBreak()
            rpStepTableUpOneStepWithMinDelay()
        # disable coils
        TableReleaseStepperPower()
    if isDown():
        print("Down")
        TableTakeStepperPower()
        for _ in range(100):
            CheckEBreak()
            rpStepTableDownOneStepWithMinDelay()
        # disable coils
        TableReleaseStepperPower()


    print("Waiting...")
    time.sleep(0.1)
