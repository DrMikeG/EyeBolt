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
    size=adafruit_ov5640.OV5640_SIZE_96X96,
)
print("print chip id")
print(cam.chip_id)

# This sets the color format for the camera to RGB. 
# The constant adafruit_ov5640.OV5640_COLOR_RGB specifies the byte ordering and format of the color data that the OV5640 camera outputs.
# In this case, the format is RGB with little-endian byte ordering.
cam.colorspace = adafruit_ov5640.OV5640_COLOR_YUV 
cam.effect = adafruit_ov5640.OV5640_SPECIAL_EFFECT_NONE
cam.white_balance = adafruit_ov5640.OV5640_WHITE_BALANCE_AUTO 
cam.flip_y = False
cam.flip_x = False
cam.night_mode = False

cam.exposure_value = 3
cam.brightness = 0
cam.saturation = 0
# In test bar mode, the camera shows color bars in the order white - yellow - cyan - green - purple - red - blue - black. 
cam.test_pattern = False


#cam.quality = 7

# Create a Bitmap object with the given fixed size. 
# Each pixel stores a value that is used to index into a corresponding palette.
# This enables differently colored sprites to share the underlying Bitmap. 
# value_count is used to minimize the memory used to store the Bitmap.

width = cam.width
height = cam.height
print(width, height)
bufSize = cam.capture_buffer_size
print(bufSize)
buf = bytearray(bufSize)
if buf is None:
    raise SystemExit("Could not allocate a bitmap")
print("capturing")
cam.capture(buf)
print("capture complete")

print("[")
separator = ','  # example separator
row_index = 1  # example row index

for row_index in range(height):

    hex_strings = []

    for col_index in range(width):
        i = row_index
        j = col_index
        bIndex = 2 * (width * j + i)
        pixel_value = (buf[bIndex+1] << 8) | buf[bIndex]
        #print(f"Pixel ({i}, {j}):({buf[bIndex+1]},{buf[bIndex]}) {pixel_value}")
        #hex_strings.append(str(hex(pixel_value))[2:4])
        hex_strings.append(hex(pixel_value))


    
    # Join the hex strings together with the separator
    result = separator.join(hex_strings)
    print("[{}],".format(result))  # prints '4321,8765,cba9,fedc'

print("]")
