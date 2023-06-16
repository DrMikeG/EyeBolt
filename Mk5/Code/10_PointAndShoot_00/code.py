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
from my_camera import init_camera
from my_motor import init_stepper_motor, stepper_fwd

class LetterBoxByteArray:
    def __init__(self, outer_width, outer_height, inner_width, inner_height):
        self.outer_width = outer_width
        self.outer_height = outer_height
        self.inner_width = inner_width
        self.inner_height = inner_height
        self.data = bytearray(inner_width * inner_height)

    def index(self, x, y):
        return y * self.inner_width + x

    def __getitem__(self, index):
        return self.data[index]

    def __setitem__(self, index, value):
        self.data[index] = value

    def __len__(self):
        return self.inner_width * self.inner_height

    def __bytes__(self):
        return bytes(self.data[:len(self)])


# Usage example
#letter_box_array = LetterBoxByteArray(640, 480, 320, 240)

# Accessing the full size but storing within the inner rectangle
#letter_box_array[100] = 42  # Setting a value at index 100 (full size index)

# Accessing the stored data within the inner rectangle
#print(letter_box_array[100])  # Accessing the value at index 100 (inner rectangle index)

print("Camera amd MiroSD test")

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
    #buffer = LetterBoxByteArray(width, height, width, height)
    quality = cam.quality
    print("Image compression factor {}".format(quality))

    if buffer is None:
        raise SystemExit("Could not allocate a bitmap")

    print("Stablise white balance")
    for _ in range(10):
        cam.capture(buffer)
        time.sleep(0.1)
    print("Done. Waiting to start")


    while True:
        
        if not button.value:
            # Button is pressed, perform your desired action here
            print("Button pressed!")
            time.sleep(2)

            dirPath = get_next_run_path()
            print("Using next available directory {}".format(dirPath))
            
            for n in range(2):
                print("Taking picture {}".format(n))
                cam.capture(buffer)
                jpgPath = dirPath + "/capture_{:0>3}_{}x{}_{}.jpg".format(n,width,height,quality)
                print("Writing image file {}".format(jpgPath))
                with open(jpgPath, "w") as f:
                    for byte in buffer:
                        f.write(byte.to_bytes(1, "big"))
                stepper_fwd(stepper_motor)
            break
        
    time.sleep(0.1)  # Small delay to debounce the button

    print("Complete")

main()