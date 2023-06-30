import cv2
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import numpy as np
import glob
import os

images = glob.glob('./Mk5/Code/10_PointAndShoot_00/run_020/*.jpg')
print ("found {} images".format(len(images)))

# Load the camera calibration data from the file
data = np.load("./Mk5/Code/10_PointAndShoot_00/ov5640_camera_calibration_2560x1440_24.npz")

# Extract the camera matrix and distortion coefficients
mtx = data['mtx']
dist = data['dist']

filename = images[0]
distorted_img = cv2.imread(filename)
# Undistort the image
image = cv2.undistort(distorted_img, mtx, dist)

# Create a figure and axes for the UI
fig, axs = plt.subplots(3, 2, figsize=(8, 8))

# Convert the image to the HSV color space
hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
cv2.imshow("Thresholded Image", hsv_image)
cv2.waitKey(0)

# Create initial threshold values
initial_thresholds = [160, 190, 85, 255, 120, 255]

# Create sliders for the color threshold values
slider_hue_min = Slider(axs[0, 0], label='Hue Min', valmin=0, valmax=255, valinit=initial_thresholds[0])
slider_hue_max = Slider(axs[0, 1], label='Hue Max', valmin=0, valmax=255, valinit=initial_thresholds[1])
slider_saturation_min = Slider(axs[1, 0], label='Saturation Min', valmin=0, valmax=255, valinit=initial_thresholds[2])
slider_saturation_max = Slider(axs[1, 1], label='Saturation Max', valmin=0, valmax=255, valinit=initial_thresholds[3])
slider_value_min = Slider(axs[2, 0], label='Value Min', valmin=0, valmax=255, valinit=initial_thresholds[4])
slider_value_max = Slider(axs[2, 1], label='Value Max', valmin=0, valmax=255, valinit=initial_thresholds[5])

# Update the thresholded image based on slider values
def update_threshold(*args):
    # Get the current slider values
    hue_min = slider_hue_min.val
    hue_max = slider_hue_max.val
    saturation_min = slider_saturation_min.val
    saturation_max = slider_saturation_max.val
    value_min = slider_value_min.val
    value_max = slider_value_max.val

    # Define the lower and upper color thresholds (in HSV)
    lower_threshold = np.array([hue_min, saturation_min, value_min])
    upper_threshold = np.array([hue_max, saturation_max, value_max])

    # Create a mask based on the color thresholds
    mask = cv2.inRange(hsv_image, lower_threshold, upper_threshold)

    # Apply the mask to the original image
    result = cv2.bitwise_and(image, image, mask=mask)

    # Display the thresholded image
    cv2.imshow("Thresholded Image", result)
    cv2.waitKey(1)

# Link the sliders to the update function
slider_hue_min.on_changed(update_threshold)
slider_hue_max.on_changed(update_threshold)
slider_saturation_min.on_changed(update_threshold)
slider_saturation_max.on_changed(update_threshold)
slider_value_min.on_changed(update_threshold)
slider_value_max.on_changed(update_threshold)

# Show the plot
plt.show()

# Close the OpenCV window
cv2.destroyAllWindows()
