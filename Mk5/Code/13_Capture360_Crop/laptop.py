import numpy as np
import cv2
import glob
import os

def pad_image(image, target_width, target_height):
    height, width = image.shape[:2]
    delta_width = target_width - width
    delta_height = target_height - height
    top = delta_height // 2
    bottom = delta_height - top
    left = delta_width // 2
    right = delta_width - left

    padded_image = cv2.copyMakeBorder(image, top, bottom, left, right, cv2.BORDER_CONSTANT, value=(0, 0, 0))

    return padded_image


def crop_image(image, crop_width, crop_height):
    height, width = image.shape[:2]
    start_x = (width - crop_width) // 2
    start_y = (height - crop_height) // 2
    end_x = start_x + crop_width
    end_y = start_y + crop_height

    cropped_image = image[start_y:end_y, start_x:end_x]

    return cropped_image

images = glob.glob('./Mk5/Code/13_Capture360_Crop/run_069/*.jpg')
print ("found {} images".format(len(images)))

# Load the camera calibration data from the file
data = np.load("./Mk5/Code/13_Capture360_Crop/ov5640_camera_calibration_2560x1440_24.npz")

# Extract the camera matrix and distortion coefficients
mtx = data['mtx']
dist = data['dist']

# Load the distorted image

makeVideo = True

if makeVideo:
    v_width = 1920
    v_height = 1080
    output_file = "./Mk5/Code/13_Capture360_Crop/run_069.mp4"  # Name of the output video file
    fps = 10  # Frames per second
    output_size = (v_width, v_height)  # Output video size
    codec = cv2.VideoWriter_fourcc(*"mp4v")  # Codec for the output video
    video_writer = cv2.VideoWriter(output_file, codec, fps, output_size)

    for filename in images:
        small_img = cv2.imread(filename)

        distorted_img = pad_image(small_img, 2560, 1440)

        # Undistort the image
        undistorted_img = cv2.undistort(distorted_img, mtx, dist)
        #cv2.imshow("Original {}".format(filename), distorted_img)
        #cv2.imshow("Corrected {}".format(filename), undistorted_img)

        cropped_image = crop_image(undistorted_img, v_width,v_height)

        video_writer.write(cropped_image)
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