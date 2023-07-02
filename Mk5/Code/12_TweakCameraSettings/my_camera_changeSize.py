import adafruit_ov5640
import busio
import board
import digitalio
import time

def change_setting(cam):

    
    print("Overriding defaults")
    # These values are valid - which means the mx and tx don't need to change to 
    # allow scaling
    # I think the ratios need to be correct though, in this cases I went 1/10th
    adafruit_ov5640._resolution_info[adafruit_ov5640.OV5640_SIZE_HD] = [240, 240, 7]  # 240x240
    # [1280, 720, _ASPECT_RATIO_16X9],  # HD
    # [2560, 1440, 0, 240, 2623, 1711, 32, 16, 2844, 1488],  # 16x9
    
    adafruit_ov5640._ratio_table[7] = [1920, 1920, 320, 0, 2543, 1951, 32, 16, 2684, 1968]  # 1x1

    print("Changing camera size")
    cam.size = adafruit_ov5640.OV5640_SIZE_HD
    #cam.size = adafruit_ov5640.OV5640_SIZE_240X240
    #cam.colorspace = adafruit_ov5640.OV5640_COLOR_GRAYSCALE
    time.sleep(1)
    print("New settings")
    time.sleep(1)
    # Can I access the resolution_info?
    (
        w,
        h,
        ratio) = adafruit_ov5640._resolution_info[cam.size]

    print("w = {}".format(w))
    print("h = {}".format(h))
    print("ratio = {}".format(ratio))

    # Can I access the values in ratio table?
    (
        max_width,
        max_height,
        start_x,
        start_y,
        end_x,
        end_y,
        offset_x,
        offset_y,
        total_x,
        total_y,
    ) = adafruit_ov5640._ratio_table[ratio]
    print("max_width = {}".format(max_width))
    print("max_height = {}".format(max_height))
    print("start_x = {}".format(start_x))
    print("start_y = {}".format(start_y))
    print("end_x = {}".format(end_x))
    print("end_y = {}".format(end_y))
    print("offset_x = {}".format(offset_x))
    print("offset_y = {}".format(offset_y))
    print("total_x = {}".format(total_x))
    print("total_y = {}".format(total_y))


def init_camera():

    # Change the values in row 0 of the _resolution_info table
    #adafruit_ov5640._resolution_info[0] = [2560,1400,8]  # Assign new values to row 0
    # Change the values in row 0 of the _ratio_table table

    #  mw,   mh,  sx,  sy,   ex,   ey, ox, oy,   tx,   ty
    #adafruit_ov5640._ratio_table[0] =  [1920, 1920, 320, 0, 2543, 1951, 32, 16, 2684, 1968]  # 1x1
    #adafruit_ov5640._ratio_table[0] = [2650, 1820, 850, 500, 1800, 1400, 32, 16, 2844, 1868]  # Assign new values to row 0

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
    size=adafruit_ov5640.OV5640_SIZE_QHDA
    )
    print("print chip id")
    print(cam.chip_id)

    #OV5640_SIZE_SVGA = 9  # 800x600
    #OV5640_SIZE_XGA = 10  # 1024x768
    #OV5640_SIZE_HD = 11  # 1280x720
    #OV5640_SIZE_SXGA = 12  # 1280x1024
    #OV5640_SIZE_UXGA = 13  # 1600x1200
    #OV5640_SIZE_QHDA = 14  # 2560x1440
    #OV5640_SIZE_WQXGA = 15  # 2560x1600
    #OV5640_SIZE_PFHD = 16  # 1088x1920
    #OV5640_SIZE_QSXGA = 17  # 2560x1920

    # This sets the color format for the camera to RGB.
    # The constant adafruit_ov5640.OV5640_COLOR_RGB specifies the byte ordering and format of the color data that the OV5640 camera outputs.
    # In this case, the format is RGB with little-endian byte ordering.
    #cam.colorspace = adafruit_ov5640.OV5640_COLOR_RGB
    
    # 2560x1440 @ 24 seems like a good starting point

    # Use JPG
    cam.quality = 24
    cam.colorspace = adafruit_ov5640.OV5640_COLOR_JPEG 

    # Use RGB
    #cam.colorspace = adafruit_ov5640.OV5640_COLOR_JPEG 
    
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

    #cam.exposure_value = 0
    #cam.brightness = 0
    #cam.saturation = 0
    #cam.contrast = 0
    # In test bar mode, the camera shows color bars in the order white - yellow - cyan - green - purple - red - blue - black.
    cam.test_pattern = False
    
    return cam