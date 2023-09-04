# SPDX-FileCopyrightText: Copyright (c) 2023 Limor Fried for Adafruit Industries
#
# SPDX-License-Identifier: Unlicense
"""
This demo is designed for the Raspberry Pi Pico. with 240x240 SPI TFT display

It shows the camera image on the LCD
"""
import time
import os
import board
import digitalio
from digitalio import DigitalInOut, Direction, Pull

import mount_sd
from my_camera_light import init_camera, change_setting
from my_motor import init_stepper_motor, stepper_fwd


print("Two photos which should line up after a 360 degree rotation")

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

    cam = init_camera(True)

    if cam is None:
        raise SystemExit("Camera not initialized")

    stepper_motor = init_stepper_motor()

    button = init_push_button()

    # Change settings
    print("Changing camera setting")
    change_setting(cam)
    time.sleep(1)

    width = cam.width
    height = cam.height
    print("Image width {} height {}".format(width, height))
    bufSize = cam.capture_buffer_size
    print("Image bytes {}".format(bufSize))
    quality = cam.quality
    print("Image compression factor {}".format(quality))
    buffer = bytearray(bufSize)

    if buffer is None:
        raise SystemExit("Could not allocate a bitmap")

    print("Stablise white balance")
    for _ in range(10):
        print(".", end='')
        cam.capture(buffer)
        time.sleep(0.1)
    print("\n", end='')

    time.sleep(1)
    print("init complete")

    print("Done. Waiting to start")

    while True:

        if not button.value:
            # Button is pressed, perform your desired action here
            print("Button pressed!")
            time.sleep(2)

            dirPath = get_next_run_path()
            print("Using next available directory {}".format(dirPath))

            # Take picture 0
            n = 0
            print("Taking picture {}".format(n))
            jpgPath = dirPath + "/capture_{:0>3}_{}x{}_{}.jpg".format(n,width,height,quality)
            cam.capture(buffer)
            print("Writing image file {}".format(jpgPath))
            total_bytes = len(buffer)
            print("Saving {} bytes".format(total_bytes))
            progress_interval = total_bytes // 20  # Update progress every 5%
            progress_list = [i * progress_interval for i in range(1, 22)]  # List representing 5%, 10%, 15%, ...
            print("....................")
            with open(jpgPath, "w") as f:
                bytes_written = 0
                for byte in buffer:
                    f.write(byte.to_bytes(1, "big"))
                    bytes_written += 1
                    if bytes_written >= progress_list[0]:
                        print("#", end="")
                        progress_list.pop(0)
            # Rotate 360

            for n in range(100):
                stepper_fwd(stepper_motor)
                time.sleep(0.1)

            # Take picture 1
            n = 1
            print("Taking picture {}".format(n))
            jpgPath = dirPath + "/capture_{:0>3}_{}x{}_{}.jpg".format(n,width,height,quality)
            cam.capture(buffer)
            print("Writing image file {}".format(jpgPath))
            total_bytes = len(buffer)
            print("Saving {} bytes".format(total_bytes))
            progress_interval = total_bytes // 20  # Update progress every 5%
            progress_list = [i * progress_interval for i in range(1, 22)]  # List representing 5%, 10%, 15%, ...
            print("....................")
            with open(jpgPath, "w") as f:
                bytes_written = 0
                for byte in buffer:
                    f.write(byte.to_bytes(1, "big"))
                    bytes_written += 1
                    if bytes_written >= progress_list[0]:
                        print("#", end="")
                        progress_list.pop(0)
            
            break # finish program

    time.sleep(0.1)  # Small delay to debounce the button

    print("Complete")

main()
