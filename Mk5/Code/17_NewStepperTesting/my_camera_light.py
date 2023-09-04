import adafruit_ov5640
import busio
import board
import digitalio
import time

def change_setting(cam):
    flip_x = False
    #https://github.com/espressif/esp32-camera/issues/229
    #you can increase the exposure by changing the vertical and horizontal timing).

    # Set for long exposure
    # 0x380C TIMING HTS default 0x0B 
    cam._write_reg_bits(0x380c,0xff,0x1f)
    # 0x380D TIMING HTS default 0x1C 
    cam._write_reg_bits(0x380d,0xff,0xff)

    #s->set_aec2(s, 0); // 0 = disable , 1 = enable
    #//night mode disable
    #cam._write_reg_bits(0x3a00,0x04,0x00)    
    
    #static int set_aec2(sensor_t *sensor, int enable)
    #{
    #    int ret = 0;
    #    ret = write_reg_bits(sensor->slv_addr, 0x3a00, 0x04, enable);
    #    if (ret == 0) {
    #        ESP_LOGD(TAG, "Set aec2 to: %d", enable);
    #        sensor->status.aec2 = enable;
    #    }
    #    return ret;
    #}
    
    #s->set_exposure_ctrl(s, 1); // 0 = disable , 1 = enable
    # 0 is false
    # 0 is !enable
    #ret = write_reg_bits(sensor->slv_addr, AEC_PK_MANUAL, AEC_PK_MANUAL_AEC_MANUALEN, !enable);
    #define AEC_PK_MANUAL   0x3503  // AEC Manual Mode Control
    #define AEC_PK_MANUAL_AEC_MANUALEN  0x01    /* Enable AEC Manual enable */
    #cam._write_reg_bits(0x3503,0x01,0x01)

    # The Auto Exposure Control (AEC) and Auto Gain Control (AGC) allows the image sensor to adjust the image brightness 
    # to a desired range by setting the proper exposure time and gain applied to the image. Besides automatic control, 
    # exposure time and gain can be set manually from external control. The related registers are listed in table 4-4.

    

    # 0x3503 AEC PK MANUAL 0x00
    # AEC Manual Mode Control
    # Bit[1]: AGC manual
    #  0: Auto enable
    #  1: Manual enable
    # Bit[0]: AEC manual
    #  0: Auto enable
    #  1: Manual enable

    # To manually change gain, first set register bit 0x3503[1] to enable manual control, then change the values in 
    # 0x350A/0x350B for the manual gain. The OV5640 has a maximum of 64x gain.

    # I want to set Bit[1] of 0x3503 to 1 for manual gain control
    cam._write_reg_bits(0x3503,0x01,0x01)
    # I want to set Bit[0] of 0x3503 to 1 for manual exposure control
#    cam._write_reg_bits(0x3503,0x02,0x01)

    #Gain is stored in reg 0x350a and Reg0x350b. If only use the gain of Reg0x350b, maximum gain of 
    #32x could be reached. It is enough for camera phone. So we don't discuss reg0x350a here.
    #Gain = reg0x350b
    #//gain = {0x350A[1:0], 0x350B[7:0]} / 16
    #0x350a Defaults to 00
    #cam._write_reg_bits(0x350A,0x03,0x00)
    cam._write_reg_bits(0x350B,0xFF,0x00)

    #s->set_gain_ctrl(s, 0); // 0 = disable , 1 = enable
    #s->set_agc_gain(s, 0); // 0 to 30
    #s->set_gainceiling(s, (gainceiling_t)6); // 0 to 6
    #s->set_bpc(s, 1); // 0 = disable , 1 = enable
    #s->set_wpc(s, 1); // 0 = disable , 1 = enable
    #s->set_raw_gma(s, 1); // 0 = disable , 1 = enable (makes much lighter and noisy)




def init_camera(useCrop):

    if useCrop:
        print("Overriding defaults")
        # These values are valid - which means the mx and tx don't need to change to 
        # allow scaling
        # I think the ratios need to be correct though, in this cases I went 1/10th
        adafruit_ov5640._resolution_info[adafruit_ov5640.OV5640_SIZE_HD] = [960, 960, 7]  # 240x240
        # For 960 - p_scale is false, binning is true, which explains why we drop into the window shift

        # [1280, 720, _ASPECT_RATIO_16X9],  # HD
        # [2560, 1440, 0, 240, 2623, 1711, 32, 16, 2844, 1488],  # 16x9
        adafruit_ov5640._ratio_table[7] = [960, 960, 800, 700, 1760, 1660, 0, 0, 2684, 1968]  # 1x1
        # changing 32 to 64 shifts the window right by 32 pixels

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
    size=adafruit_ov5640.OV5640_SIZE_HD
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
    cam.quality = 6
    cam.colorspace = adafruit_ov5640.OV5640_COLOR_JPEG 

    # Use RGB
    #cam.colorspace = adafruit_ov5640.OV5640_COLOR_JPEG 
    
    cam.effect = adafruit_ov5640.OV5640_SPECIAL_EFFECT_NONE
    #cam.effect = adafruit_ov5640.OV5640_SPECIAL_EFFECT_NEGATIVE
    cam.white_balance = adafruit_ov5640.OV5640_WHITE_BALANCE_SUNNY

    #OV5640_WHITE_BALANCE_AUTO = 0
    #OV5640_WHITE_BALANCE_SUNNY = 1
    #OV5640_WHITE_BALANCE_FLUORESCENT = 2
    #OV5640_WHITE_BALANCE_CLOUDY = 3
    #OV5640_WHITE_BALANCE_INCANDESCENT = 4

    cam.flip_y = False
    cam.flip_x = False
    cam.night_mode = False

    #cam.exposure_value = 3
    #cam.brightness = 3
    #cam.saturation = 0
    #cam.contrast = 1
    # In test bar mode, the camera shows color bars in the order white - yellow - cyan - green - purple - red - blue - black.
    cam.test_pattern = False
    

    # Set for long exposure
    # 0x380C TIMING HTS default 0x0B 
    cam._write_reg_bits(0x380c,0xff,0x1f)
    # 0x380D TIMING HTS default 0x1C 
    cam._write_reg_bits(0x380d,0xff,0xff)



    print("Resolution info:")
    (
        w,
        h,
        ratio) = adafruit_ov5640._resolution_info[cam.size]
    print("w = {}".format(w))
    print("h = {}".format(h))
    print("ratio = {}".format(ratio))

    print("\nRegistry values\n")
    p_size = cam.size
    p_width, p_height, p_ratio = adafruit_ov5640._resolution_info[p_size]
    (
        p_max_width,
        p_max_height,
        p_start_x,
        p_start_y,
        p_end_x,
        p_end_y,
        p_offset_x,
        p_offset_y,
        p_total_x,
        p_total_y,
    ) = adafruit_ov5640._ratio_table[p_ratio]

    print("max_width = {}".format(p_max_width))
    print("max_height = {}".format(p_max_height))
    print("start_x = {}".format(p_start_x))
    print("start_y = {}".format(p_start_y))
    print("end_x = {}".format(p_end_x))
    print("end_y = {}".format(p_end_y))
    print("offset_x = {}".format(p_offset_x))
    print("offset_y = {}".format(p_offset_y))
    print("total_x = {}".format(p_total_x))
    print("total_y = {}".format(p_total_y))
    p_binning = (p_width <= p_max_width // 2) and (p_height <= p_max_height // 2)
    print("p_binning = {}".format(p_binning))
    p_scale = not (
        (p_width == p_max_width and p_height == p_max_height)
        or (p_width == p_max_width // 2 and p_height == p_max_height // 2)
    )
    print("p_scale = {}".format(p_scale))

    print("p_write_addr_reg(_X_ADDR_ST_H, {}, {})".format(p_start_x,p_start_y))
    print("p_write_addr_reg(_X_ADDR_END_H, {},{})".format(p_end_x, p_end_y))
    print("p_write_addr_reg(_X_OUTPUT_SIZE_H, {},{})".format(p_width, p_height))

    if not p_binning:
        print("not binning")
        print("p_write_addr_reg(_X_TOTAL_SIZE_H, {},{})".format(p_total_x, p_total_y))
        print("p_write_addr_reg(_X_OFFSET_H, {},{})".format(p_offset_x, p_offset_y))
    else:
        print("binning")
        if p_width > 920:
            print("p_width > 920")
            print("p_write_addr_reg(_X_TOTAL_SIZE_H, {}, {})".format(p_total_x - 200,p_total_y // 2))
        else:
            print("p_width < 920")
            print("p_write_addr_reg(_X_TOTAL_SIZE_H, {}, {})".format(2060,p_total_y // 2))
            print("p_write_addr_reg(_X_OFFSET_H, {}, {})".format(p_offset_x // 2,p_offset_y // 2))

    print("p_write_reg_bits(_ISP_CONTROL_01, 0x20, {})".format(p_scale))

    # other recommended settings:
    #s->set_brightness(s, 0); // -2 to 2
    #s->set_contrast(s, -2); // -2 to 2
    #s->set_saturation(s, -2); // -2 to 2
    #s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    
    
    #s->set_whitebal(s, 1); // 0 = disable , 1 = enable
    #s->set_awb_gain(s, 1); // 0 = disable , 1 = enable
    #static int set_awb_gain_dsp(sensor_t *sensor, int enable)
    #{
        #define CTRL0               0xC2
        #define CTRL1               0xC3
    #    sensor->status.awb_gain = enable;
    #    return set_reg_bits(sensor, BANK_DSP, CTRL1, 2, 1, enable?1:0);
    #                   uint8_t offset, uint8_t mask, uint8_t value
    #    return set_reg_bits(sensor, 0       , 0xC3 , 2, 1, enable?1:0);
    # Mast 2, offset 1
    #}


    #s->set_wb_mode(s, 0); // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)

    #s->set_exposure_ctrl(s, 1); // 0 = disable , 1 = enable
    #s->set_aec2(s, 0); // 0 = disable , 1 = enable
    #//s->set_ae_level(s, 2); // -2 to 2
    #//s->set_aec_value(s, 400); // 0 to 1200
    #s->set_gain_ctrl(s, 0); // 0 = disable , 1 = enable
    #s->set_agc_gain(s, 0); // 0 to 30
    #s->set_gainceiling(s, (gainceiling_t)6); // 0 to 6
    #s->set_bpc(s, 1); // 0 = disable , 1 = enable
    #s->set_wpc(s, 1); // 0 = disable , 1 = enable
    #s->set_raw_gma(s, 1); // 0 = disable , 1 = enable (makes much lighter and noisy)
    #s->set_lenc(s, 0); // 0 = disable , 1 = enable
    #s->set_hmirror(s, 0); // 0 = disable , 1 = enable
    #s->set_vflip(s, 0); // 0 = disable , 1 = enable
    #s->set_dcw(s, 0); // 0 = disable , 1 = enable
    #s->set_colorbar(s, 0); // 0 = disable , 1 = enable


    # And finally, being able to save high quality images (if the image is getting corrupted)
    #cam._write_register(0xff,0xff,0x00);//banksel
    #cam._write_register(0xd3,5)#;//jpg clock

    return cam
