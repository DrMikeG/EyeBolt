# SPDX-FileCopyrightText: Copyright (c) 2023 Limor Fried for Adafruit Industries
#
# SPDX-License-Identifier: Unlicense
"""
This demo is designed for the Raspberry Pi Pico. with 240x240 SPI TFT display
"""
import time
import os
import board
import digitalio
from digitalio import DigitalInOut, Direction, Pull

import mount_sd
from my_camera import init_camera, change_setting
from my_motor import init_stepper_motor, stepper_fwd


print("Take 2 photos, changing the setting in between")

def init_push_button():
    button_pin = board.GP6
    button = digitalio.DigitalInOut(button_pin)
    button.direction = digitalio.Direction.INPUT
    button.pull = digitalio.Pull.UP
    return button

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

    if cam is None:
        raise SystemExit("Camera not initialized")

    stepper_motor = init_stepper_motor()

    button = init_push_button()

    print("init complete")

    width = cam.width
    height = cam.height
    print("Image width {} height {}".format(width, height))
    bufSize = cam.capture_buffer_size
    print("Image bytes {}".format(bufSize))
    buffer = bytearray(bufSize)
    quality = cam.quality
    print("Image compression factor {}".format(quality))

    if buffer is None:
        raise SystemExit("Could not allocate a bitmap")

    print("Stablise white balance")
    for _ in range(10):
        cam.capture(buffer)
        time.sleep(0.1)
    time.sleep(1)
    print("Done. Waiting to start")

    while True:
        
        if not button.value:
            # Button is pressed, perform your desired action here
            print("Button pressed!")

            dirPath = get_next_run_path()
            print("Using next available directory {}".format(dirPath))

            # Capture image with default settings
            print("Taking picture 0")
            cam.capture(buffer)
            jpgPath = dirPath + "/capture_0.jpg"
            print("Writing image file {}".format(jpgPath))
            with open(jpgPath, "w") as f:
                for byte in buffer:
                    f.write(byte.to_bytes(1, "big"))
            
            # Change settings
            print("Changing camera setting")
            change_setting(cam)
            time.sleep(1)
            
            # Capture image with modified settings
            print("Taking picture 1")
            cam.capture(buffer)
            jpgPath = dirPath + "/capture_1.jpg"
            print("Writing image file {}".format(jpgPath))
            with open(jpgPath, "w") as f:
                for byte in buffer:
                    f.write(byte.to_bytes(1, "big"))

            break
        
    time.sleep(0.1)  # Small delay to debounce the button

    print("Complete")

main()