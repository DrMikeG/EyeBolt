import cv2
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import numpy as np
import glob
import os


width = 1024
height = 768
quality = 8

for e in range(-3,4):
    for c in range(-3,4):
        for b in range(-4,5):
            print("/capture_{}_{}_{}.jpg".format(e,c,b))