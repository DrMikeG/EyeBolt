import numpy as np
import cv2
import glob

images = glob.glob('./Mk5/Code/10_PointAndShoot_00/run_019/*.jpg')
print ("found {} images".format(len(images)))

# Load the camera calibration data from the file
data = np.load("./Mk5/Code/10_PointAndShoot_00/ov5640_camera_calibration_2560x1440_24.npz")

# Extract the camera matrix and distortion coefficients
mtx = data['mtx']
dist = data['dist']

# Load the distorted image

output_file = "./Mk5/Code/10_PointAndShoot_00/run_019.mp4"  # Name of the output video file
fps = 30  # Frames per second
output_size = (2560, 1440)  # Output video size
codec = cv2.VideoWriter_fourcc(*"mp4v")  # Codec for the output video
video_writer = cv2.VideoWriter(output_file, codec, fps, output_size)

for filename in images:
    distorted_img = cv2.imread(filename)
    # Undistort the image
    undistorted_img = cv2.undistort(distorted_img, mtx, dist)
    #cv2.imshow("Original {}".format(filename), distorted_img)
    #cv2.imshow("Corrected {}".format(filename), undistorted_img)
    video_writer.write(undistorted_img)
    #cv2.waitKey(30)  # Display each frame for 30 milliseconds
    
    #cv2.imwrite(filename.replace(".jpg","_corrected.jpg"), undistorted_img)
    #cv2.waitKey(0)
    #break
video_writer.release()
cv2.destroyAllWindows()

    
    