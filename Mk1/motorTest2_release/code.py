# This test was to convince myself that I should be releasing the coils after advancing the table
import time
import board
from digitalio import DigitalInOut, Direction, Pull
from adafruit_motor import stepper

print("Stepper test")

led = DigitalInOut(board.LED)
led.direction = Direction.OUTPUT
led.value = True

def blink(times):
    for _ in range(times):
        led.value = False
        time.sleep(0.1)
        led.value = True
        time.sleep(0.1)

# Mode button setup
button = DigitalInOut(board.GP3)
button.direction = Direction.INPUT
button.pull = Pull.UP
mode = -1  # track state of button mode

# Stepper motor setup
DELAY = 0.01  # fastest is ~ 0.004, 0.01 is still very smooth, gets steppy after that
STEPS = 220  # this is a full 360º
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

def StepTableDownOneStepWithDelay():
    stepper_motor.onestep(direction=stepper.FORWARD,style=stepper.DOUBLE) # FORWARDs is table down
    time.sleep(0.002)

for _ in range(10):
        blink(10)
        StepTableDownOneStepWithDelay()
        stepper_motor.release()
