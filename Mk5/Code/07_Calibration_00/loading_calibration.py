import numpy as np
import cv2
#import yam
import glob

images = glob.glob('./Mk5/Code/07_Calibration_00/run_0*/*.jpg')
print ("found {} images".format(len(images)))
filename = images[13]

#filename = './Mk5/Code/07_Calibration_00/test_img.jpg'



# Load the camera calibration data from the file
data = np.load("./Mk5/Code/07_Calibration_00/ov5640_camera_calibration.npz")

# Extract the camera matrix and distortion coefficients
mtx = data['mtx']
dist = data['dist']

# Load the distorted image

distorted_img = cv2.imread(filename)

# Undistort the image
undistorted_img = cv2.undistort(distorted_img, mtx, dist)
cv2.imshow("Original {}".format(filename), distorted_img)
cv2.imshow("Corrected {}".format(filename), undistorted_img)
cv2.imwrite(filename.replace(".jpg","_corrected.jpg"), undistorted_img)
cv2.waitKey(0)