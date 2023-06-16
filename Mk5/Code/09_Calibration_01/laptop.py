import numpy as np
import cv2
#import yam
import glob

# termination criteria
criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 90, 0.0001)

########################################Blob Detector##############################################

# Setup SimpleBlobDetector parameters.
blobParams = cv2.SimpleBlobDetector_Params()
# Change thresholds
blobParams.minThreshold = 0 # Changing this to zero improved things
blobParams.maxThreshold = 255

# Filter by Area.
blobParams.filterByArea = True
blobParams.minArea = 18     # minArea may be adjusted to suit for your experiment
blobParams.maxArea = 2500   # maxArea may be adjusted to suit for your experiment

# Filter by Circularity
blobParams.filterByCircularity = True
blobParams.minCircularity = 0.1

# Filter by Convexity
blobParams.filterByConvexity = True
blobParams.minConvexity = 0.9

# Filter by Inertia
blobParams.filterByInertia = True
blobParams.minInertiaRatio = 0.05

# Made no difference
blobParams.filterByColor: False
blobParams.blobColor: 0


# Create a detector with the parameters
blobDetector = cv2.SimpleBlobDetector_create(blobParams)

###################################################################################################

###################################################################################################

# Original blob coordinates, supposing all blobs are of z-coordinates 0
# And, the distance between every two neighbour blob circle centers is 72 centimetres
# In fact, any number can be used to replace 72.
# Namely, the real size of the circle is pointless while calculating camera calibration parameters.
objp = np.zeros((44, 3), np.float32)
objp[0]  = (0  , 0  , 0)
objp[1]  = (0  , 72 , 0)
objp[2]  = (0  , 144, 0)
objp[3]  = (0  , 216, 0)
objp[4]  = (36 , 36 , 0)
objp[5]  = (36 , 108, 0)
objp[6]  = (36 , 180, 0)
objp[7]  = (36 , 252, 0)
objp[8]  = (72 , 0  , 0)
objp[9]  = (72 , 72 , 0)
objp[10] = (72 , 144, 0)
objp[11] = (72 , 216, 0)
objp[12] = (108, 36,  0)
objp[13] = (108, 108, 0)
objp[14] = (108, 180, 0)
objp[15] = (108, 252, 0)
objp[16] = (144, 0  , 0)
objp[17] = (144, 72 , 0)
objp[18] = (144, 144, 0)
objp[19] = (144, 216, 0)
objp[20] = (180, 36 , 0)
objp[21] = (180, 108, 0)
objp[22] = (180, 180, 0)
objp[23] = (180, 252, 0)
objp[24] = (216, 0  , 0)
objp[25] = (216, 72 , 0)
objp[26] = (216, 144, 0)
objp[27] = (216, 216, 0)
objp[28] = (252, 36 , 0)
objp[29] = (252, 108, 0)
objp[30] = (252, 180, 0)
objp[31] = (252, 252, 0)
objp[32] = (288, 0  , 0)
objp[33] = (288, 72 , 0)
objp[34] = (288, 144, 0)
objp[35] = (288, 216, 0)
objp[36] = (324, 36 , 0)
objp[37] = (324, 108, 0)
objp[38] = (324, 180, 0)
objp[39] = (324, 252, 0)
objp[40] = (360, 0  , 0)
objp[41] = (360, 72 , 0)
objp[42] = (360, 144, 0)
objp[43] = (360, 216, 0)
###################################################################################################

# Arrays to store object points and image points from all the images.
objpoints = [] # 3d point in real world space
imgpoints = [] # 2d points in image plane.

#cap = cv2.VideoCapture(0)

images = glob.glob('./Mk5/Code/09_Calibration_01/run_0*/*.jpg')

height = 0
width = 0

print ("found {} images".format(len(images)))
for fname in images:

    found = 0
    img = cv2.imread(fname)
    height, width, _ = img.shape
    ret, corners = cv2.findCirclesGrid(img, (4,11), None, flags = cv2.CALIB_CB_ASYMMETRIC_GRID,  blobDetector = blobDetector)   # Find the circle grid
    
    if ret == True:
        print("Found {} corners".format(len(corners)))
        objpoints.append(objp)  # Certainly, every loop objp is the same, in 3D.
        # Draw and display the corners.
        imgpoints.append(corners)
        img = cv2.drawChessboardCorners(img, (4,11), corners, ret)
        found += 1
        #cv2.imshow("{}".format(fname), img) # display
        #cv2.waitKey(0)
        #cv2.destroyAllWindows()
    else:
        print("Failed to find corners in {}".format(fname))

# When everything done, release the capture
#cap.release()
cv2.destroyAllWindows()

#ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, img.shape[::-1], None, None)
ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, [width,height], None, None)

# It's very important to transform the matrix to list.
data = {'camera_matrix': np.asarray(mtx).tolist(), 'dist_coeff': np.asarray(dist).tolist()}
#with open("calibration.yaml", "w") as f:
    #yaml.dump(data, f)

np.savez("./Mk5/Code/09_Calibration_01/ov5640_camera_calibration_2560x1440_24.npz", mtx=mtx, dist=dist)

# Load the distorted image
filename = images[0]
distorted_img = cv2.imread(filename)

# Undistort the image
undistorted_img = cv2.undistort(distorted_img, mtx, dist)
cv2.imshow("Original {}".format(filename), distorted_img)
cv2.imshow("Corrected {}".format(filename), undistorted_img)
cv2.waitKey(0)

print(data)