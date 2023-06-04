import numpy as np
import cv2
#import yam
import glob

# termination criteria
criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.0001)

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

images = glob.glob('./Mk5/Code/07_Calibration_00/run_0*/*.jpg')
found = 0
print ("found {} images".format(len(images)))
for fname in images:

    img = cv2.imread(fname)

    ret, corners = cv2.findCirclesGrid(img, (4,11), None, flags = cv2.CALIB_CB_ASYMMETRIC_GRID,  blobDetector = blobDetector)   # Find the circle grid
    
    if ret == True:
        found += 1
    else:
        gray_version_of_img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        keypoints_in_img = blobDetector.detect(gray_version_of_img) # Detect blobs.
        # Draw detected blobs as red circles. This helps cv2.findCirclesGrid() .
        img_marked_up_with_keypoints = cv2.drawKeypoints(img, keypoints_in_img, np.array([]), (0,255,0), cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)
        cv2.imshow("Found {} keypoints in image {}".format(len(keypoints_in_img),fname), img_marked_up_with_keypoints)
        cv2.waitKey(0)
        #cv2.imwrite(fname+".bad.jpg", img_marked_up_with_keypoints)
        break
        

print("success in {} out of {}".format(found,len(images)))