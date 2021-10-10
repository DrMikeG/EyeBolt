# This will be my main script going forward
# Notes on progress:
# all button inputs tested


import time
import board
import digitalio
import pwmio
from digitalio import DigitalInOut, Direction, Pull
from adafruit_motor import stepper, servo


# Servo control
servoFAST = 1
servoSLOW = 2
servoRetract = 145
servoHome = 143
servoLimit = 102
servoAngleToEdgeOfTable = 134

servoCurrentTarget = 143
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
        stepper_motor.release()
        # Don't use MoveServo as that calls CheckEBreak
        my_servo.angle = servoHome
        while True:
            if button14.value == False:
                print("OK button releases panic")
                break
            time.sleep(0.1)

def MoveServo(newTargetAngle,speed):
    global servoCurrentTarget
    CheckEBreak()
    #print("MoveServoTo: "+str(newTargetAngle))
    if newTargetAngle <= servoRetract:
        if newTargetAngle >= servoLimit:
            if speed == servoFAST:
                servoCurrentTarget = newTargetAngle
                my_servo.angle = servoCurrentTarget
                CheckEBreak()
            elif speed == servoSLOW:
                step = (newTargetAngle - servoCurrentTarget) / 20
                for _ in range(20):
                    CheckEBreak()
                    thisStep = servoCurrentTarget + step
                    if thisStep <= servoRetract:
                        if thisStep >= servoLimit:
                            my_servo.angle = thisStep
                            time.sleep(0.1)
                CheckEBreak()
                servoCurrentTarget = newTargetAngle
                my_servo.angle = servoCurrentTarget
    time.sleep(0.25)

def StepTableUpOneStepWithDelay():
    stepper_motor.onestep(direction=stepper.BACKWARD,style=stepper.DOUBLE) # BACKWARDs is table up
    time.sleep(0.002)

def StepTableDownOneStepWithDelay():
    stepper_motor.onestep(direction=stepper.FORWARD,style=stepper.DOUBLE) # FORWARDs is table down
    time.sleep(0.002)

def TableIsAtUpperStop():
    return button13.value == False


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
        time.sleep(0.004) # extra delay to slow speed
    print("Table retract from upper limit - done")

    print("Table seek upper limit")
    # Move slowly up until upper limit switch presses
    while not TableIsAtUpperStop():
        CheckEBreak()
        #Move stepper clockwise (up) slow
        StepTableUpOneStepWithDelay()
        time.sleep(0.004) # extra delay to slow speed
    print("Table seek upper limit - done")

    print("Table Homed")
    return True


def MovePokerToNearTableEdge():
    CheckEBreak()
    print("MovePokerToNearTableEdge")
    if PokerHasTouched():
        return False # unexpected
    MoveServo(servoHome,servoSLOW)
    hasTouched = PokerHasNotTouched()
    print("has touched: "+str(hasTouched))
    return hasTouched


def FullyRetractPoker():
    CheckEBreak()
    print("FullyRetractPoker")
    # Change servo to value X
    MoveServo(servoRetract,servoFAST)
    isReset = PokerIsReset()
    #print("isReset: "+str(isReset))
    return isReset

def PokerHasNotTouched():
    # Sensor open means not touched
    return photoSensorPin2.value == False # False is open

def PokerHasTouched():
    # Sensor closed means not touched
    return photoSensorPin2.value == True # True is closed

def PokerIsReset():
    return PokerHasNotTouched()



def TakeMeasurement():
    global servoCurrentTarget

    print("Measuring")

    ok = FullyRetractPoker()
    if not ok : return False

    # ignore limit for now
    print("Measuring - starting")
    measureAngle = servoCurrentTarget    
    hasTouched = False

    while not hasTouched:
        measureAngle = measureAngle - 1
        MoveServo(measureAngle,servoFAST)
        hasTouched = PokerHasTouched()
        if measureAngle == servoLimit:
            break

    print("Measuring - done")

    ok = FullyRetractPoker()
    if not ok : return False

    return ok, hasTouched, measureAngle


def ConfirmFirstMeasureToSideOfTable():
    print("ConfirmFirstMeasureToSideOfTable()")
    ok, hasTouched, measureAngle = TakeMeasurement()

    if not ok:
         print("Error in TakeMeasurement()")
         return False

    if not hasTouched:
         print("Error Confirming first measurement to side of table - did not touch")
         return False

    print("Table measured at :"+str(measureAngle))

    if (abs(measureAngle - servoAngleToEdgeOfTable) < 2):
        ok = True
        print("ConfirmFirstMeasureToSideOfTable() - done")
    else:
        ok = False
        print("ConfirmFirstMeasureToSideOfTable() - failed")
    
    return ok

def PerformScan():
    print("PerformScan")
    # ConfirmFirstMeasureToSideOfTable

    ok = ConfirmFirstMeasureToSideOfTable()
    if not ok : return False

    # TurnAndMeasure
    # 100 x 25 steps didn't even clear the table
    for stepCount in range(100):
        for _ in range(25):
            CheckEBreak()
            StepTableDownOneStepWithDelay()
        ok, hasTouched, measureAngle = TakeMeasurement()
        if not ok:
            print("Error in TakeMeasurement()")
            return False
        print("Step %i touched %s angle %s" % (stepCount, str(hasTouched),str(measureAngle)))

    print("PerformScan - done")
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
    time.sleep(1.0)
    print("Fully retract and confirm sensor reset - done")

    ok = MovePokerToNearTableEdge()
    if not ok : return False
    time.sleep(1.0)
    print("Stage Poker and confirm sensor available - done")

    ok = FullyRetractPoker()
    if not ok : return False
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

    ok = PokerHomeCycle()

    if ok:
        ok = TableHomeCycle()

    if ok:
        print("Press confirm to begin scan?")
        while True:
            CheckEBreak()
            # Wait for initialise, button 14...
            if button14.value == False:
                ok = PerformScan()
                # Looping here...

    if ok:
         ok = PokerHomeCycle()
    if ok:
        ok = TableHomeCycle()

    print("Done!")


CheckEBreak()
my_servo.angle = servoHome
servoCurrentTarget = servoHome

while True:

    # Wait for initialise, button 14...
    if button14.value == False:
        PrepareThenPerformScan()

    if button12.value == False:
        print("Lower limit switch is pressed")
        time.sleep(0.5)

    if button13.value == False:
        print("Upper limit switch is pressed")
        time.sleep(0.5)

    if photoSensorPin2.value == True:
        print("Photo sensor closed")
        time.sleep(0.5)




