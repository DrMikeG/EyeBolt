import time
import os
import adafruit_sdcard
import board
import busio
import digitalio
import storage
from digitalio import DigitalInOut, Direction, Pull
from adafruit_motor import stepper

print("MiroSD breakout with 28by test")
#adafruit_sdcard.mpy
#adafruit_bus_device

# Use any pin that is not taken by SPI
SD_CS = board.GP5

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

led = DigitalInOut(board.LED)
led.direction = Direction.OUTPUT
led.value = True

def blink(times):
    for _ in range(times):
        led.value = False
        time.sleep(0.1)
        led.value = True
        time.sleep(0.1)

# Connect to the card and mount the filesystem.
spi = busio.SPI(clock=board.GP2, MOSI=board.GP3, MISO=board.GP4)
cs = digitalio.DigitalInOut(SD_CS)
sdcard = adafruit_sdcard.SDCard(spi, cs)
vfs = storage.VfsFat(sdcard)
storage.mount(vfs, "/sd")

# Use the filesystem as normal! Our files are under /sd

# This helper function will print the contents of the SD

def print_directory(path, tabs=0):
    for file in os.listdir(path):
        stats = os.stat(path + "/" + file)
        filesize = stats[6]
        isdir = stats[0] & 0x4000

        if filesize < 1000:
            sizestr = str(filesize) + " by"
        elif filesize < 1000000:
            sizestr = "%0.1f KB" % (filesize / 1000)
        else:
            sizestr = "%0.1f MB" % (filesize / 1000000)

        prettyprintname = ""
        for _ in range(tabs):
            prettyprintname += "   "
        prettyprintname += file
        if isdir:
            prettyprintname += "/"
        print('{0:<40} Size: {1:>10}'.format(prettyprintname, sizestr))

        # recursively print directory contents
        if isdir:
            print_directory(path + "/" + file, tabs + 1)



def stepper_fwd():
    print("stepper forward")
    for _ in range(STEPS):
        stepper_motor.onestep(direction=stepper.FORWARD)
        time.sleep(DELAY)
    stepper_motor.release()


while True:
        blink(10)
        stepper_fwd()
        time.sleep(0.2)
        print("Files on filesystem:")
        print("====================")
        print_directory("/sd")
        time.sleep(0.5)



