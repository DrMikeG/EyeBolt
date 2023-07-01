import adafruit_ov5640
import busio
import board
import digitalio

def change_setting(cam):

    # Change the camera from
    #  [2560, 1440, _ASPECT_RATIO_16X9], # QHD
    # to
    #  [1280, 720, _ASPECT_RATIO_16X9],  # HD
    # (Same aspect ratio but much smaller)
    size=adafruit_ov5640.OV5640_SIZE_QHDA
    # self._size = size
    # self._set_size_and_colorspace()


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

    cam.quality = 24

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

    cam.exposure_value = -3
    cam.brightness = 0
    cam.saturation = -2
    cam.contrast = -2
    # In test bar mode, the camera shows color bars in the order white - yellow - cyan - green - purple - red - blue - black.
    cam.test_pattern = False
    
    return cam