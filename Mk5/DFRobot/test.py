import time
import board
from digitalio import DigitalInOut, Direction, Pull
from adafruit_motor import stepper

print("Stepper test")

DELAY = 0.005  # fastest is ~ 0.004, 0.01 is still very smooth, gets steppy after that
STEPS = 20 # (360/18) = 20 * 100 steps per rev

def init_stepper_motor():
    print("initialising Stepper motor")
    # It has a 1/64 reduction gear set and therefore moves in precise 512 steps per revolution
    # Stepper motor setup
    coils = (
        DigitalInOut(board.GP22),  # A1
        DigitalInOut(board.GP26),  # A2
        DigitalInOut(board.GP27),  # B1
        DigitalInOut(board.GP28),  # B2
    )
    for coil in coils:
        coil.direction = Direction.OUTPUT

    stepper_motor = stepper.StepperMotor(
        coils[0], coils[1], coils[2], coils[3], microsteps=None
    )
    return stepper_motor

def stepper_fwd(stepper_motor):
    print("stepper forward")
    for _ in range(STEPS):
        stepper_motor.onestep(direction=stepper.FORWARD)
        time.sleep(DELAY)
    stepper_motor.release()

stepper1 = init_stepper_motor()

for n in range(100):
    stepper_fwd(stepper1)
    time.sleep(0.2)

