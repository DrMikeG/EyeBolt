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
mode = -1  # track state of button mode

# It has a 1/64 reduction gear set and therefore moves in precise 512 steps per revolution
# Stepper motor setup
DELAY = 0.005  # fastest is ~ 0.004, 0.01 is still very smooth, gets steppy after that
STEPS = 512  # this is a full 360º
coils = (
    DigitalInOut(board.GP2),  # A1
    DigitalInOut(board.GP3),  # A2
    DigitalInOut(board.GP4),  # B1
    DigitalInOut(board.GP5),  # B2
)
for coil in coils:
    coil.direction = Direction.OUTPUT

stepper_motor = stepper.StepperMotor(
    coils[0], coils[1], coils[2], coils[3], microsteps=None
)


def stepper_fwd():
    print("stepper forward")
    for _ in range(STEPS):
        stepper_motor.onestep(direction=stepper.FORWARD)
        time.sleep(DELAY)
    stepper_motor.release()


while True:
        blink(10)
        stepper_fwd()
