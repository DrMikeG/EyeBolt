
import adafruit_ov5640
import busio
import board
import digitalio
import time

def init_camera():

    print("Overriding defaults")
    adafruit_ov5640._resolution_info[adafruit_ov5640.OV5640_SIZE_HD] = [960, 960, 7]  # 240x240
    adafruit_ov5640._ratio_table[7] = [960, 960, 800, 700, 1760, 1660, 0, 0, 2684, 1968]  # 1x1

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
    size=adafruit_ov5640.OV5640_SIZE_HD
    )

    # Use JPG
    cam.quality = 6
    cam.colorspace = adafruit_ov5640.OV5640_COLOR_JPEG 
    cam.effect = adafruit_ov5640.OV5640_SPECIAL_EFFECT_NONE
    cam.white_balance = adafruit_ov5640.OV5640_WHITE_BALANCE_SUNNY
    cam.flip_y = False
    cam.flip_x = True
    cam.night_mode = False
    cam.test_pattern = False
    
    #https://github.com/espressif/esp32-camera/issues/229
    #you can increase the exposure by changing the vertical and horizontal timing).
    # TIMING HTS 
    # TIMING VTS 
    # 0x380C TIMING HTS 0x0B RW 
    # 0x380D TIMING HTS 0x1C 
    # 0x380E TIMING VTS 0x07
    # 0x380F TIMING VTS 0xB0
    # Set for long exposure
    cam._write_reg_bits(0x380c,0xff,0x1f)     # 0x380C TIMING HTS default 0x0B 
    cam._write_reg_bits(0x380d,0xff,0xff)      # 0x380D TIMING HTS default 0x1C 

    #If you want to increase the exposure even further, you can do something like this:
    #s->set_reg(s,0x380e,0xff,0x2f);
    #s->set_reg(s,0x380f,0xff,0xff);
    cam._write_reg_bits(0x380e,0xff,0x2f) # 0x380E TIMING VTS default 0x07 
    cam._write_reg_bits(0x380f,0xff,0xff) # 0x380F TIMING VTS default 0xB0 

    
    # To manually change exposure value, you must first set both 0x3503[0], where 0x3503[0] enables manual exposure  control.
    # I want to set Bit[1] of 0x3503 to 1 for manual gain control
    cam._write_reg_bits(0x3503,0x01,0x01)
    # I want to set Bit[0] of 0x3503 to 1 for manual exposure control
    cam._write_reg_bits(0x3503,0x02,0x01)

    # To manually change gain, first set register bit 0x3503[1] to enable manual control, then change the values in 
    #' 0x350A/0x350B for the manual gain. The OV5640 has a maximum of 64x gain.
    #Gain is stored in reg 0x350a and Reg0x350b. If only use the gain of Reg0x350b, maximum gain of 
    #32x could be reached. It is enough for camera phone. So we don't discuss reg0x350a here.
    #Gain = reg0x350b
    #//gain = {0x350A[1:0], 0x350B[7:0]} / 16
    #0x350a Defaults to 00
    #cam._write_reg_bits(0x350A,0x03,0x00)
    cam._write_reg_bits(0x350B,0xFF,0x00)

    return cam