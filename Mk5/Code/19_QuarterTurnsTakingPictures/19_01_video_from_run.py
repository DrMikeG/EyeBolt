import numpy as np
import cv2
import glob
import os
import open3d as o3d

## General manipulation

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

def make_single_video(images,mtx,dist,output_file):
    font = cv2.FONT_HERSHEY_SIMPLEX
    font_scale = 1.0
    color = (255, 255, 255)  # BGR format
    thickness = 2
    v_width = 1920
    v_height = 1080
    fps = 6  # Frames per second
    output_size = (v_width, v_height)  # Output video size
    codec = cv2.VideoWriter_fourcc(*"mp4v")  # Codec for the output video
    video_writer = cv2.VideoWriter(output_file, codec, fps, output_size)

    all_frames = []

    for filename in images:
        small_img = cv2.imread(filename)

        distorted_img = pad_image(small_img, 2560, 1440)

        # Undistort the image
        undistorted_img = cv2.undistort(distorted_img, mtx, dist)
        #cv2.imshow("Original {}".format(filename), distorted_img)
        #cv2.imshow("Corrected {}".format(filename), undistorted_img)

        # Extract numbers using split()
        # Extract the final substring after 'capture'
        substring = filename.split('capture_')[-1].split('.')[0]
        frame_number = substring.split('_')[0]
        
        # Add the text to the image
        cv2.putText(undistorted_img, frame_number, (800, 250) , font, font_scale, color, thickness)
        #cv2.imshow("Corrected {}".format(filename), undistorted_img)
        #cv2.waitKey(0)
        #break
        cropped_image = crop_image(undistorted_img, v_width,v_height)

        all_frames.append(cropped_image)

        video_writer.write(cropped_image)

    for l in range(4):
        print("Looping frames x{}".format(l))        
        for i in all_frames:
            video_writer.write(i)

    print("Writing video {}".format(output_file))
    video_writer.release()
    cv2.destroyAllWindows()

run = 29
dark_images = glob.glob('./Mk5/Code/19_QuarterTurnsTakingPictures/run_0{}*/*.jpg'.format(run))
video_output_file = "./Mk5/Code/19_QuarterTurnsTakingPictures/run_0{}.mp4".format(run)  # Name of the output video file

# Load the camera calibration data from the file
data = np.load("./Mk5/Code/13_Capture360_Crop/ov5640_camera_calibration_2560x1440_24.npz")
# Extract the camera matrix and distortion coefficients
mtx = data['mtx']
dist = data['dist']

make_single_video(dark_images,mtx,dist,video_output_file)