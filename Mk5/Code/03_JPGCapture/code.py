# SPDX-FileCopyrightText: Copyright (c) 2023 Limor Fried for Adafruit Industries
#
# SPDX-License-Identifier: Unlicense
"""
This demo is designed for the Raspberry Pi Pico. with 240x240 SPI TFT display

It shows the camera image on the LCD
"""
import time
import busio
import board
import digitalio
import adafruit_ov5640
import displayio

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
    size=adafruit_ov5640.OV5640_SIZE_SVGA
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

# Create a Bitmap object with the given fixed size.
# Each pixel stores a value that is used to index into a corresponding palette.
# This enables differently colored sprites to share the underlying Bitmap.
# value_count is used to minimize the memory used to store the Bitmap.

width = cam.width
height = cam.height
print(width, height)
bufSize = cam.capture_buffer_size
print(bufSize)
buffer = bytearray(bufSize)
if buffer is None:
    raise SystemExit("Could not allocate a bitmap")

for _ in range(10):
    cam.capture(buffer)
    time.sleep(0.1)


print("[",end='')
for b in range(bufSize):
    print(hex(buffer[b]), end=',')
print("]",end='')
