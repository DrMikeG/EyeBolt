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

# Panic?
button15 = digitalio.DigitalInOut(board.GP15)
button15.switch_to_input(pull=digitalio.Pull.UP)

photoSensorPin2 = digitalio.DigitalInOut(board.GP3)
photoSensorPin2.switch_to_input(pull=digitalio.Pull.UP)

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

global_X = 0
global_Z = 0
global_scanLength = 0


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
    stepper_motor2.release()
    while True:
        # Don't have the option to continue
        #if thumbSwitch.value == False:
        #    print("OK button releases panic")
        #    break
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

def isOKPressed():
    return thumbSwitch.value == False

def BeamIsNotBroken():
    # Sensor open means not touched
    return photoSensorPin2.value == False # False is open

def BeamIsBroken():
    # Sensor closed means not touched
    return photoSensorPin2.value == True # True is closed

## Motor Movements (RP = require power) ###################
# You have to take correct motor out of standby before calling these methods

tableMotorDelay = 0.002
beamMotorDelay = 0.0008

def rpStepTableUpOneStepWithMinDelay():
    AssertTableHasMotorPower() # This might be too slow to check here?
    tableMotor.onestep(direction=stepper.FORWARD,style=stepper.DOUBLE) # BACKWARDs is table up
    # 0.002 is min and max (sweat spot)
    time.sleep(tableMotorDelay)

def rpStepTableDownOneStepWithMinDelay():
    AssertTableHasMotorPower() # This might be too slow to check here?
    tableMotor.onestep(direction=stepper.BACKWARD,style=stepper.DOUBLE) # FORWARDs is table down
    time.sleep(tableMotorDelay)

def rpStepCarriageBackOneStepWithMinDelay():
    AssertBeltHasMotorPower() # This might be too slow to check here?
    beltMotor.onestep(direction=stepper.BACKWARD,style=stepper.DOUBLE) # BACKWARDs is table up
    # min is 0.0005
    # max is 0.002
    time.sleep(beamMotorDelay)

def rpStepCarriageForwardOneStepWithMinDelay():
    AssertBeltHasMotorPower() # This might be too slow to check here?
    beltMotor.onestep(direction=stepper.FORWARD,style=stepper.DOUBLE) # FORWARDs is table down
    time.sleep(beamMotorDelay)

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

## Track position on two axes ############################

def incX(value = 1):
    global global_X
    global_X+=value

def decX(value = 1):
    global global_X
    global_X-=value

def zeroX():
    global global_X
    print("Zeroing X (from %u)" % (global_X))
    global_X=0

def currentX():
    global global_X
    return global_X

def incZ(value = 1):
    global global_Z
    global_Z+=value

def decZ(value = 1):
    global global_Z
    global_Z-=value

def zeroZ():
    global global_Z
    print("Zeroing Z (from %u)" % (global_Z))
    global_Z=0

def currentZ():
    global global_Z
    return global_Z

## Move one or more elements (with position tracking) ############################
    
def MoveBeamInX(deltaInSteps):

    if (deltaInSteps == 1):
        CheckEBreak()
        rpStepCarriageBackOneStepWithMinDelay()
        incX()
        return
    
    if (deltaInSteps > 0):
        for _ in range(deltaInSteps):
            CheckEBreak()
            rpStepCarriageBackOneStepWithMinDelay()
        incX(deltaInSteps)
        return

    if (deltaInSteps < 0):
        for _ in range(-deltaInSteps):
            CheckEBreak()
            rpStepCarriageForwardOneStepWithMinDelay()    
        decX(-deltaInSteps)
        return

def MoveTableInZ(deltaInSteps):

    if (deltaInSteps > 0):
        for _ in range(deltaInSteps):
            CheckEBreak()
            rpStepTableDownOneStepWithMinDelay()
            incZ()

    if (deltaInSteps < 0):
        for _ in range(-deltaInSteps):
            CheckEBreak()
            rpStepTableUpOneStepWithMinDelay()
            decZ()            

## Scanning processes ############################

def IdentifyScanLength():
    
    stepSizeForCalibration = 10

    print("Please move beam to one end of the scan range and confirm")

    BeltTakeStepperPower()    
    while True:        
        if isLeft():
            MoveBeamInX(+stepSizeForCalibration)
            
        if isRight():
            MoveBeamInX(-stepSizeForCalibration)

        if isOKPressed():
            time.sleep(0.5)
            break
    BeltReleaseStepperPower()
    
    zeroX()

    print("Please move beam to other end of the scan range and confirm")

    # Need to count values now
    BeltTakeStepperPower()    
    while True:        
        if isLeft():
            MoveBeamInX(+stepSizeForCalibration)
            
        if isRight():
            MoveBeamInX(-stepSizeForCalibration)

        if isOKPressed():
            time.sleep(0.5)
            break

    _scanLength = currentX()
    print("Scan length of %u steps" %(_scanLength))

    print("Returning to zero")
    MoveBeamInX(-_scanLength)

    BeltReleaseStepperPower()

    return _scanLength


def IdentifyTableZero():

    print("You can move the beam now, it will rezero at start of scan")
    print("Move the beam somewhere useful, and then confirm")
    stepSizeForCalibration = 10
    BeltTakeStepperPower()    
    while True:        
        if isLeft():
            MoveBeamInX(+stepSizeForCalibration)
            
        if isRight():
            MoveBeamInX(-stepSizeForCalibration)

        if isOKPressed():
            time.sleep(0.5)
            break
    BeltReleaseStepperPower()

    print("Please move table up until it reaches the beam and confirm")

    # 2500 steps per revolution
    stepsPerClick = 500

    TableTakeStepperPower()
    while True:        
        if isUp():            
            MoveTableInZ(-stepsPerClick)
            
        if isDown():
            MoveTableInZ(stepsPerClick)

        if isOKPressed():
            time.sleep(0.5)
            break

    TableReleaseStepperPower()

    zeroZ()    

def PerformScan():
    global global_scanLength

    BeltTakeStepperPower()
    print("Returning to zero")
    MoveBeamInX(-currentX())
    BeltReleaseStepperPower()
    # Repeat 50
        # Sweep X from 0 to global_scanLength
        # Rotate chunk of 25 steps
        # Sweep X from global_scanLength to 0
        # Rotate chunk of 25 steps
    for _ in range(50):
        BeltTakeStepperPower()
        MoveBeamInX(global_scanLength)
        BeltReleaseStepperPower()
        
        TableTakeStepperPower()
        MoveTableInZ(25)
        TableReleaseStepperPower()

        BeltTakeStepperPower()
        MoveBeamInX(-global_scanLength)
        BeltReleaseStepperPower()
        
        TableTakeStepperPower()
        MoveTableInZ(25)
        TableReleaseStepperPower()


def PrepareThenPerformScan():
    global global_scanLength

    ok = True

    print("Zeroing X out at %u" %(currentX()))
    zeroX()

    # Rather than try to criss-cross the boundary in one pass
    # lets just plow back and forth and calculate an average

    numberOfStepsObjectDefWithinFromZero = 3000

    delay = 5
    plowTimes = 10
    minStep = 10

    objectDetected = BeamIsBroken()

    for i in range(plowTimes):

        print("Loop %u of %u" %(i,plowTimes))

        changeAt = []

        BeltTakeStepperPower()
        
        # Move one way and then the other
        for i in range(numberOfStepsObjectDefWithinFromZero / minStep):
            MoveBeamInX(minStep)
            if not objectDetected == BeamIsBroken():
                #print("Detect changed at %u" %(currentX()))
                changeAt.append(currentX())
                objectDetected = not objectDetected
        
        for c in changeAt:
            print("Detect changed at %u" %(c))
        changeAt.clear()

        for i in range(numberOfStepsObjectDefWithinFromZero / minStep):
            MoveBeamInX(-minStep)
            if not objectDetected == BeamIsBroken():
                #print("Detect changed at %u" %(currentX()))
                changeAt.append(currentX())
                objectDetected = not objectDetected

        for c in changeAt:
            print("Detect changed at %u" %(c))
        changeAt.clear()

        BeltReleaseStepperPower()

    print("Done!")

CheckEBreak()

print("Check beam alignment")
print("Move back and forward. Will pause when beam broken")

BeltTakeStepperPower()

while True:

    CheckEBreak()

    if BeamIsBroken():
        print("Beam broken!")
        time.sleep(0.1)

    if isLeft():
        MoveBeamInX(50)

    if isRight():
        MoveBeamInX(-50)

    if isUp():
        print("Up")
        BeltReleaseStepperPower()
        TableTakeStepperPower()
        for _ in range(100):
            CheckEBreak()
            rpStepTableUpOneStepWithMinDelay()
        TableReleaseStepperPower()
        BeltTakeStepperPower()


    if isDown():
        print("Down")
        BeltReleaseStepperPower()
        TableTakeStepperPower()
        for _ in range(100):
            CheckEBreak()
            rpStepTableDownOneStepWithMinDelay()
        TableReleaseStepperPower()
        BeltTakeStepperPower()

    #print("Waiting...")
    #time.sleep(0.1)

BeltReleaseStepperPower()

