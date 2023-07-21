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


def find_red_pixels(image, threshold):
    # Convert BGR image to HSV
    hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

    # Define lower and upper threshold values for red (HSV color space)
    lower_red = np.array([100, 000, 000], dtype=np.uint8)
    upper_red = np.array([179, 255, 255], dtype=np.uint8)

    # Create a binary mask for red pixels
    red_mask = cv2.inRange(hsv_image, lower_red, upper_red)
    # Create a white image of the same size as the original image
    white_image = np.ones_like(image) * 255
 
    # Apply the mask to the original image
    result = cv2.bitwise_and(image, image, mask=red_mask)
    #cv2.imshow("Mask applied {}".format(filename), result)
    #cv2.imshow("Original applied {}".format(filename), image)
    #cv2.waitKey(0)

    # Find the x, y coordinates of red pixels
    y_coords, x_coords = np.where(red_mask > 0)

    return x_coords, y_coords

def write_xyz_file(x_coords, y_coords, z_value, output_file):
    with open(output_file, 'w') as f:
        for x, y in zip(x_coords, y_coords):
            f.write(f"{x} {y} {z_value}\n")
    print(f"Coordinates for {z_value} written to {output_file}")



images = glob.glob('./Mk5/Code/13_Capture360_Crop/run_071/*.jpg')
print ("found {} images".format(len(images)))

# Load the camera calibration data from the file
data = np.load("./Mk5/Code/13_Capture360_Crop/ov5640_camera_calibration_2560x1440_24.npz")

# Extract the camera matrix and distortion coefficients
mtx = data['mtx']
dist = data['dist']

# Load the distorted image

makeVideo = False

if makeVideo:
    v_width = 1920
    v_height = 1080
    output_file = "./Mk5/Code/13_Capture360_Crop/run_071.mp4"  # Name of the output video file
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
    
processFirstImageTest = True

# Specify the output file name
output_file = "./Mk5/Code/14_Open3d_Tut/run_71_output.xyz"

if processFirstImageTest:

    hue_min = 0
    saturation_min = 0
    value_min = 0
    hue_max = hue_min + 254
    saturation_max = saturation_min + 100
    value_max = value_min + 100

    z_value = 0
    for filename in images:
        v_width = 1280
        v_height = 1024
        small_img = cv2.imread(filename) # 900x900
        distorted_img = pad_image(small_img, 2560, 1440) # 2560x1440
        # Undistort the image
        undistorted_img = cv2.undistort(distorted_img, mtx, dist) # 2560x1440
        cropped_image = crop_image(undistorted_img, v_width,v_height)

        red_threshold = 10

        # Find red pixels
        x_coords, y_coords = find_red_pixels(cropped_image, red_threshold)

        # Display the results
        print(f"Number of red pixels: {len(x_coords)}")
        #for x, y in zip(x_coords, y_coords):
            #print(f"Pixel coordinates: ({x}, {y})")

        
        # Write the coordinates to the .xyz file
        write_xyz_file(x_coords, y_coords,z_value,output_file)

        z_value += 1.0

        break
    #cv2.destroyAllWindows()