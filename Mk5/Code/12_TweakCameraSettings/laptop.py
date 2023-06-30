import numpy as np
import cv2
import glob
import os

images = glob.glob('./Mk5/Code/11_CheckExposure/run_024/*.jpg')

print ("found {} images".format(len(images)))

# Load the camera calibration data from the file
data = np.load("./Mk5/Code/11_CheckExposure/ov5640_camera_calibration_2560x1440_24.npz")

# Extract the camera matrix and distortion coefficients
mtx = data['mtx']
dist = data['dist']

# Load the distorted image

makeVideo = True

if makeVideo:

    # Define the font properties
    font = cv2.FONT_HERSHEY_SIMPLEX
    font_scale = 1.0
    color = (255, 255, 255)  # BGR format
    thickness = 2

    output_file = "./Mk5/Code/11_CheckExposure/run_024_exp_minus3.mp4"  # Name of the output video file
    fps = 3  # Frames per second
    output_size = (2560, 1440)  # Output video size
    codec = cv2.VideoWriter_fourcc(*"mp4v")  # Codec for the output video
    video_writer = cv2.VideoWriter(output_file, codec, fps, output_size)


    # Extract numbers and create a tuple for sort key
    keyed_filenames = [(filename, tuple(int(num) for num in filename.split('capture_')[-1].split('.')[0].split('_'))) for filename in images]

    # Sort the filenames based on the tuple sort key
    sorted_filenames = sorted(keyed_filenames, key=lambda x: x[1], reverse=True)

    for filename,_ in sorted_filenames:
        distorted_img = cv2.imread(filename)
        # Undistort the image
        undistorted_img = cv2.undistort(distorted_img, mtx, dist)
        
        # Extract numbers using split()
        # Extract the final substring after 'capture'
        substring = filename.split('capture_')[-1].split('.')[0]

        # Extract numbers using split()
        numbers = [int(num) for num in substring.split('_')]
        
        # Assign numbers to variables
        e, c, b = numbers
        if not e == -3:
            continue

        print("Filename {} exposure={}, constrast={} brightness={}".format(filename,e,c,b))
        
        exposureText = "exposure={}".format(e)
        brightnessText = "brightness={}".format(b)
        constrastText = "constrast={}".format(c)
        brightnessText = "brightness={}".format(b)
        # Add the text to the image
        cv2.putText(undistorted_img, exposureText, (750, 150) , font, font_scale, color, thickness)
        cv2.putText(undistorted_img, brightnessText, (750, 200) , font, font_scale, color, thickness)
        cv2.putText(undistorted_img, constrastText, (750, 250) , font, font_scale, color, thickness)
        #cv2.imshow("Original {}".format(filename), distorted_img)
        #cv2.imshow("Corrected {}".format(filename), undistorted_img)
        video_writer.write(undistorted_img)
        #cv2.waitKey(30)  # Display each frame for 30 milliseconds
        
        #cv2.imwrite(filename.replace(".jpg","_corrected.jpg"), undistorted_img)
        #cv2.waitKey(0)
        
        #break
    video_writer.release()
cv2.destroyAllWindows()

saveUndistorted = False

if saveUndistorted:

    input_folder = './Mk5/Code/10_PointAndShoot_00/run_020'
    output_folder = input_folder + '_corrected'

    # Check if the output folder exists
    if not os.path.exists(output_folder):
        # Create the output folder
        os.makedirs(output_folder)
        print("Output folder created:", output_folder)
    else:
        print("Output folder already exists:", output_folder)

    for filename in images:
        distorted_img = cv2.imread(filename)
        undistorted_img = cv2.undistort(distorted_img, mtx, dist)
        cv2.imwrite(filename.replace(input_folder,output_folder).replace(".jpg","_corrected.jpg"), undistorted_img)
    
    
processFirstImageTest = False

if processFirstImageTest:

    hue_min = 0
    saturation_min = 140
    value_min = 75
    hue_max = hue_min + 254
    saturation_max = saturation_min + 100
    value_max = value_min + 100

    for filename in images:
        distorted_img = cv2.imread(filename)
        # Undistort the image
        undistorted_img = cv2.undistort(distorted_img, mtx, dist)
        #cv2.imshow("Corrected {}".format(filename), undistorted_img)
        #cv2.imwrite(filename.replace(".jpg","_corrected.jpg"), undistorted_img)
        # Convert the image to the HSV color space
        hsv_image = cv2.cvtColor(undistorted_img, cv2.COLOR_BGR2HSV)

        # Define the lower and upper color thresholds (in HSV)
        lower_threshold = np.array([hue_min, saturation_min, value_min])
        upper_threshold = np.array([hue_max, saturation_max, value_max])

        # Create a mask based on the color thresholds
        mask = cv2.inRange(hsv_image, lower_threshold, upper_threshold)

        # Apply the mask to the original image
        result = cv2.bitwise_and(undistorted_img, undistorted_img, mask=mask)

        cv2.imshow("Processed {}".format(filename), result)

        cv2.waitKey(0)
        break
    cv2.destroyAllWindows()