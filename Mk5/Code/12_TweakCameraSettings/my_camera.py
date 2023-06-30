import adafruit_ov5640
import busio
import board
import digitalio

def change_setting(cam):
    #cam.flip_x = True
    reg20 = reg21 = reg4514 = reg4514_test = 0
    if cam.colorspace == adafruit_ov5640.OV5640_COLOR_JPEG:
        reg21 |= 0x20

    if cam._binning:
        reg20 |= 1
        reg21 |= 1
        reg4514_test |= 4
    else:
        reg20 |= 0x40

    if cam._flip_y:
        reg20 |= 0x06
        reg4514_test |= 1

    #if self._flip_x:
    reg21 |= 0x06
    reg4514_test |= 2

    if reg4514_test == 0:
        reg4514 = 0x88
    elif reg4514_test == 1:
        reg4514 = 0x00
    elif reg4514_test == 2:
        reg4514 = 0xBB
    elif reg4514_test == 3:
        reg4514 = 0x00
    elif reg4514_test == 4:
        reg4514 = 0xAA
    elif reg4514_test == 5:
        reg4514 = 0xBB
    elif reg4514_test == 6:
        reg4514 = 0xBB
    elif reg4514_test == 7:
        reg4514 = 0xAA

    _TIMING_TC_REG20 = 0x3820
    _TIMING_TC_REG21 = 0x3821
    cam._write_register(_TIMING_TC_REG20, reg20)
    cam._write_register(_TIMING_TC_REG21, reg21)
    cam._write_register(0x4514, reg4514)

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

    #cam.exposure_value = 3
    #cam.brightness = 3
    #cam.saturation = 4
    #cam.constrast = 3
    # In test bar mode, the camera shows color bars in the order white - yellow - cyan - green - purple - red - blue - black.
    cam.test_pattern = False
    
    return cam