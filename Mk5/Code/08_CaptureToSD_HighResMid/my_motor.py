import board
from digitalio import DigitalInOut, Direction, Pull
from adafruit_motor import stepper


def init_stepper_motor():
    print("initialising Stepper motor")
    # It has a 1/64 reduction gear set and therefore moves in precise 512 steps per revolution
    # Stepper motor setup
    DELAY = 0.005  # fastest is ~ 0.004, 0.01 is still very smooth, gets steppy after that
    STEPS = 512  # this is a full 360º
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