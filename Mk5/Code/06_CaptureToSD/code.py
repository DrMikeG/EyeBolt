# SPDX-FileCopyrightText: Copyright (c) 2023 Limor Fried for Adafruit Industries
#
# SPDX-License-Identifier: Unlicense
"""
This demo is designed for the Raspberry Pi Pico. with 240x240 SPI TFT display

It shows the camera image on the LCD
"""
import time
import os
import busio
import board
import digitalio
import adafruit_ov5640
import adafruit_sdcard
import storage
import displayio
from digitalio import DigitalInOut, Direction, Pull
from adafruit_motor import stepper

print("Camera amd MiroSD test")

def init_push_button():
    button_pin = board.GP6
    button = digitalio.DigitalInOut(button_pin)
    button.direction = digitalio.Direction.INPUT
    button.pull = digitalio.Pull.UP
    return button

def init_camera():
    print("initialising Camera")
    print("construct bus")
    bus = busio.I2C(board.GP9, board.GP8)
    print("construct camera")
    reset = digitalio.DigitalInOut(board.GP10)
    cam = adafruit_ov5640.OV5640(
    bus,
    data_pins=(
        board.GP12,
        board.GP13,
        board.GP14,
        board.GP15,
        board.GP16,
        board.GP17,
        board.GP18,
        board.GP19,
    ),
    clock=board.GP11,
    vsync=board.GP7,
    href=board.GP21,
    mclk=board.GP20,
    shutdown=None,
    reset=reset,
    size=adafruit_ov5640.OV5640_SIZE_XGA
    )
    print("print chip id")
    print(cam.chip_id)

    # This sets the color format for the camera to RGB.
    # The constant adafruit_ov5640.OV5640_COLOR_RGB specifies the byte ordering and format of the color data that the OV5640 camera outputs.
    # In this case, the format is RGB with little-endian byte ordering.
    cam.colorspace = adafruit_ov5640.OV5640_COLOR_JPEG
    cam.effect = adafruit_ov5640.OV5640_SPECIAL_EFFECT_NONE
    #cam.effect = adafruit_ov5640.OV5640_SPECIAL_EFFECT_NEGATIVE
    cam.white_balance = adafruit_ov5640.OV5640_WHITE_BALANCE_AUTO

    #OV5640_WHITE_BALANCE_AUTO = 0
    #OV5640_WHITE_BALANCE_SUNNY = 1
    #OV5640_WHITE_BALANCE_FLUORESCENT = 2
    #OV5640_WHITE_BALANCE_CLOUDY = 3
    #OV5640_WHITE_BALANCE_INCANDESCENT = 4

    cam.flip_y = False
    cam.flip_x = False
    #cam.night_mode = False

    #cam.exposure_value = 3
    #cam.brightness = 3
    #cam.saturation = 4
    #cam.constrast = 3
    # In test bar mode, the camera shows color bars in the order white - yellow - cyan - green - purple - red - blue - black.
    cam.test_pattern = False
    cam.quality = 8
    return cam

def init_SD():
    #adafruit_sdcard.mpy
    #adafruit_bus_device
    print("initialising SD Card file system")
    # Use any pin that is not taken by SPI
    SD_CS = board.GP5
    # Connect to the card and mount the filesystem.
    print("construct spi bus")
    spi = busio.SPI(clock=board.GP2, MOSI=board.GP3, MISO=board.GP4)
    cs = digitalio.DigitalInOut(SD_CS)
    sdcard = adafruit_sdcard.SDCard(spi, cs)
    print("construct VfsFat storage")
    vfs = storage.VfsFat(sdcard)
    storage.mount(vfs, "/sd")

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


def does_file_exist(filename):
    try:
        status = os.stat(filename)
        file_exists = True
    except OSError:
        file_exists = False
    return file_exists

def does_directory_exist(filename):
    try:
        status = os.stat(filename)
        isdir = status[0] & 0x4000
    except OSError:
        isdir = False
    return isdir

def get_next_run_path():
    basePath = "/sd/run_"
    threeDigitNumberAsString = "000"
    # Keep generating numbers until find an unused folder name
    # and then create the folder
    # and return the path
    for i in range(1000):
        path = "/sd/run_{0:0>3}".format(i)
        if not does_directory_exist(path):
            os.mkdir(path)
            return path
    print("Card too full! (over 999 directories)")

def main():

    cam = init_camera()

    init_SD()

    stepper_motor = init_stepper_motor()

    button = init_push_button()

    print("init complete")

    # Create a Bitmap object with the given fixed size.
    # Each pixel stores a value that is used to index into a corresponding palette.
    # This enables differently colored sprites to share the underlying Bitmap.
    # value_count is used to minimize the memory used to store the Bitmap.

    width = cam.width
    height = cam.height
    print("Image width {} height {}".format(width, height))
    bufSize = cam.capture_buffer_size
    print("Image bytes {}".format(bufSize))
    buffer = bytearray(bufSize)
    if buffer is None:
        raise SystemExit("Could not allocate a bitmap")

    print("Stablise white balance")
    for _ in range(10):
        cam.capture(buffer)
        time.sleep(0.1)


    while True:
        
        if not button.value:
            # Button is pressed, perform your desired action here
            print("Button pressed!")
    
            dirPath = get_next_run_path()
            print("Using next available directory {}".format(dirPath))
            
            for n in range(5):
                print("Taking picture {}".format(n))
                cam.capture(buffer)
                jpgPath = dirPath + "/capture_{0:0>3}.jpg".format(n)
                print("Writing image file {}".format(jpgPath))
                with open(jpgPath, "w") as f:
                    for byte in buffer:
                        f.write(byte.to_bytes(1, "big"))
            break
        
    time.sleep(0.1)  # Small delay to debounce the button

    print("Complete")

main()