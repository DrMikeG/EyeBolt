import board, busio, digitalio, adafruit_sdcard, storage

#adafruit_sdcard.mpy
#adafruit_bus_device
print("initialising SD Card file system")
# Use any pin that is not taken by SPI
SD_CS = board.GP5
# Connect to the card and mount the filesystem.
print("construct spi bus")
spi = busio.SPI(clock=board.GP2, MOSI=board.GP3, MISO=board.GP4)
cs = digitalio.DigitalInOut(SD_CS)
sdcard = adafruit_sdcard.SDCard(spi, cs)
print("construct VfsFat storage")
vfs = storage.VfsFat(sdcard)
storage.mount(vfs, "/sd")