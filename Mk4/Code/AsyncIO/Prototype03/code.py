import time
import board
import digitalio
from digitalio import DigitalInOut, Direction, Pull
from adafruit_motor import stepper
import analogio
import asyncio
import countio


class Sweep:
    """Simple class to hold an interval value. Use .value to to read or write."""

    def __init__(self):
        self.beam_broke = False
        self.current_x = 0
        self.previous_x = 0
        self.transitions = []

sweep = Sweep()

# Thumbstick
thumbSwitch = digitalio.DigitalInOut(board.GP22)
thumbSwitch.switch_to_input(pull=digitalio.Pull.UP)

thumbLeftRight = analogio.AnalogIn(board.GP26)
thumbUpDown = analogio.AnalogIn(board.GP27)

# Panic?
# This is registered for interrupt rather than button now
#button15 = digitalio.DigitalInOut(board.GP15)

#photoSensorPin2 = digitalio.DigitalInOut(board.GP3)
#photoSensorPin2.switch_to_input(pull=digitalio.Pull.UP)

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

#global_X = 0
global_Z = 0
global_scanLength = 0


## LED ##########################

async def blink(pin, interval, count):  # Don't forget the async!
    with digitalio.DigitalInOut(pin) as led:
        led.switch_to_output(value=False)
        while True:
        #for _ in range(count):
            led.value = True
            await asyncio.sleep(interval)  # Don't forget the await!
            led.value = False
            await asyncio.sleep(interval)  # Don't forget the await!

## Sensor inputs ##########################

def Panic():
    # Park Servo
    print("PANIC")
    stepper_motor1.release()
    stepper_motor2.release()
    print("OK button releases panic")
    while True:
        # Don't have the option to continue
        if thumbSwitch.value == False:            
            break
        time.sleep(0.1)

async def catch_abort_interrupt():
    with countio.Counter(board.GP15, edge=countio.Edge.FALL,pull=digitalio.Pull.UP) as interrupt:
        while True:
            if interrupt.count > 0:
                interrupt.reset()
                Panic()
            # Let another task run.
            await asyncio.sleep(0)


async def poll_beam(current_sweep):
     with digitalio.DigitalInOut(board.GP3) as beam:
        beam.direction = Direction.INPUT
        beam.pull = Pull.UP
        while True:
            # Assume True beam not broken?
            if beam.value != current_sweep.beam_broke:
                fromStr ="Object Detected"
                toStr = "Object not Detected"
                if beam.value == False:
                    fromStr ="Object not Detected"
                    toStr = "Object Detected"
                print("Beam break state changed from %s to %s at %u,%u" % (fromStr,toStr,current_sweep.current_x,current_sweep.previous_x))                
                current_sweep.beam_broke = not current_sweep.beam_broke
                
                #current_sweep.transitions.append((current_sweep.current_x,current_sweep.previous_x))
            # Let another task run.
            await asyncio.sleep(0.05)


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

## Motor Movements (RP = require power) ###################
# You have to take correct motor out of standby before calling these methods

tableMotorDelay = 0.003 #0.002 
beamMotorDelay = 0.0008

def rpStepTableUpOneStepWithMinDelay():
    AssertTableHasMotorPower() # This might be too slow to check here?
    tableMotor.onestep(direction=stepper.FORWARD,style=stepper.DOUBLE) # BACKWARDs is table up
    # 0.002 is min and max (sweat spot)
#    await asyncio.sleep(tableMotorDelay)

def rpStepTableDownOneStepWithMinDelay():
    AssertTableHasMotorPower() # This might be too slow to check here?
    tableMotor.onestep(direction=stepper.BACKWARD,style=stepper.DOUBLE) # FORWARDs is table down
#    await asyncio.sleep(tableMotorDelay)

def rpStepCarriageBackOneStepWithMinDelay():
    AssertBeltHasMotorPower() # This might be too slow to check here?
    beltMotor.onestep(direction=stepper.FORWARD,style=stepper.DOUBLE) # BACKWARDs is table up
    # min is 0.0005
    # max is 0.002
    #await asyncio.sleep(beamMotorDelay)

def rpStepCarriageForwardOneStepWithMinDelay():
    AssertBeltHasMotorPower() # This might be too slow to check here?
    beltMotor.onestep(direction=stepper.BACKWARD,style=stepper.DOUBLE) # FORWARDs is table down
    #await asyncio.sleep(beamMotorDelay)

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
    global sweep
    sweep.previous_x = sweep.current_x
    sweep.current_x += value
    #global_X+=value

def zeroX():
    #global global_X
    global sweep
    print("Zeroing X (from %u)" % (sweep.current_x))
    sweep.previous_x = 0
    sweep.current_x = 0
    #global_X=0

def currentX():
    global sweep
    return sweep.current_x

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
    
async def MoveBeamInX(deltaInSteps):

    if (deltaInSteps > 0):
        for _ in range(deltaInSteps):
            incX()
            rpStepCarriageBackOneStepWithMinDelay()
            await asyncio.sleep(beamMotorDelay)
        return

    if (deltaInSteps < 0):
        for _ in range(-deltaInSteps):
            incX(-1)
            rpStepCarriageForwardOneStepWithMinDelay()    
            await asyncio.sleep(beamMotorDelay)        
        return

async def MoveTableInZ(deltaInSteps):

    if (deltaInSteps > 0):
        for _ in range(deltaInSteps):
            incZ()
            rpStepTableDownOneStepWithMinDelay()
            await asyncio.sleep(tableMotorDelay)
            incZ()

    if (deltaInSteps < 0):
        for _ in range(-deltaInSteps):
            decZ()            
            rpStepTableUpOneStepWithMinDelay()
            await asyncio.sleep(tableMotorDelay)
            

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

    numberOfStepsObjectDefWithinFromZero = 8000

    delay = 5
    plowTimes = 10
    minStep = 10

    objectDetected = BeamIsBroken()

    for i in range(plowTimes):

        print("Loop %u of %u starting from %u" %(i,plowTimes,currentX()))

        changeAt = []

        BeltTakeStepperPower()
        
        nChunks = numberOfStepsObjectDefWithinFromZero / minStep
        # Move one way and then the other
        for i in range(nChunks):
            MoveBeamInX(minStep)
            if not objectDetected == BeamIsBroken():
                #print("Detect changed at %u" %(currentX()))
                changeAt.append(currentX())
                objectDetected = not objectDetected
        
        for c in changeAt:
            print("Detect changed at %u" %(c))
        changeAt.clear()

        for i in range(nChunks):
            MoveBeamInX(-minStep)
            #if not objectDetected == BeamIsBroken():
                #print("Detect changed at %u" %(currentX()))
            #    changeAt.append(currentX())
            #    objectDetected = not objectDetected

        #for c in changeAt:
        #    print("Detect changed at %u" %(c))
        #changeAt.clear()

        BeltReleaseStepperPower()

    print("Done!")




async def runScannerTask(sweep):

    print("Bear bones calibration")
    print("Position laser and part - first scan direction will be toward the belt motor")
    print("Then press ok!")

    while True:

        # Allow time for other threads
        await asyncio.sleep(0.05)

        if isOKPressed():
            zeroX()
            await asyncio.sleep(0.2)
            #time.sleep(0.2)
            #PrepareThenPerformScan()

        if isLeft():
            print("Left!")
            BeltTakeStepperPower()   
            # Create Async task, which starts it running
            # Wait for async task to complete before moving on
            await asyncio.gather(asyncio.create_task(MoveBeamInX(6000))) 
            BeltReleaseStepperPower()
            await asyncio.sleep(0.2)

        if isRight():
            print("Right!")
            BeltTakeStepperPower()
            await asyncio.gather(asyncio.create_task(MoveBeamInX(-6000))) 
            BeltReleaseStepperPower()
            await asyncio.sleep(0.2)

        if isUp():
            print("Up")
            TableTakeStepperPower()
            await asyncio.gather(asyncio.create_task(MoveTableInZ(-100)))
            TableReleaseStepperPower()
            await asyncio.sleep(0.2)

        if isDown():
            print("Down")
            TableTakeStepperPower()
            await asyncio.gather(asyncio.create_task(MoveTableInZ(100)))
            TableReleaseStepperPower()
            await asyncio.sleep(0.2)

async def main():
    
    abort_task = asyncio.create_task(catch_abort_interrupt())# Runs forever
    
    
    beam_task = asyncio.create_task(poll_beam(sweep))# Runs forever
    #led_task = asyncio.create_task(blink(board.LED, 0.5, 10)) # Runs forever
    scanner_task = asyncio.create_task(runScannerTask(sweep)) # Runs until complete
    
    await asyncio.gather(scanner_task)  # If the scanner task ends, all task end
    print("done")


asyncio.run(main())