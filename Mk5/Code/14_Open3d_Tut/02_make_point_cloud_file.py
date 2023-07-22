import numpy as np
import cv2
import glob
import os
import open3d as o3d
from datum import makeDatumAsPointCloud

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

## Processing stages

def make_single_video(images,mtx,dist,output_file):
    
    v_width = 1920
    v_height = 1080
    fps = 60  # Frames per second
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

        cropped_image = crop_image(undistorted_img, v_width,v_height)

        all_frames.append(cropped_image)

        video_writer.write(cropped_image)
        #cv2.waitKey(30)  # Display each frame for 30 milliseconds
        
        #cv2.imwrite(filename.replace(".jpg","_corrected.jpg"), undistorted_img)
        #cv2.waitKey(0)
        #break

    # Don't have the seem at the end of the video
    for i in all_frames:
        video_writer.write(i)
    for i in all_frames:
        video_writer.write(i)        

    video_writer.release()
    cv2.destroyAllWindows()

def make_side_by_side_video(dark_images,light_images,mtx,dist,output_file):
    
    v_width = 1920
    v_height = 1080
    print ("Making side by side dark/light video @{}x{} to {}".format(v_width,v_height,output_file))
    fps = 30  # Frames per second
    output_size = (v_width, v_height)  # Output video size
    codec = cv2.VideoWriter_fourcc(*"mp4v")  # Codec for the output video
    video_writer = cv2.VideoWriter(output_file, codec, fps, output_size)

    if not len(dark_images) == len(light_images):        
        raise ValueError("Incompatible number of images found: dark {} light {}".format(len(dark_images),len(light_images)))        

    all_frames = []

    frame_number = 0
    for d_filename, l_filename in zip(dark_images, light_images):
        print("Processing combined frame number {} from {}".format(frame_number,d_filename))
        d_small_img = cv2.imread(d_filename)
        d_distorted_img = pad_image(d_small_img, 2560, 1440)
        l_small_img = cv2.imread(l_filename)
        l_distorted_img = pad_image(l_small_img, 2560, 1440)

        d_undistorted_img = cv2.undistort(d_distorted_img, mtx, dist)
        l_undistorted_img = cv2.undistort(l_distorted_img, mtx, dist)

        d_cropped_image = crop_image(d_undistorted_img, int(v_width/2),v_height)
        l_cropped_image = crop_image(l_undistorted_img, int(v_width/2),v_height)

        # Check if the images have the same height
        if d_cropped_image.shape[0] != l_cropped_image.shape[0]:
            raise ValueError("Both images should have the same height.")

        # Combine the images side by side into one 1800x900 image
        combined_img = np.concatenate((d_cropped_image, l_cropped_image), axis=1)
        
        # Define the text to be written and the position to place it
        
        text = f"{frame_number}"
        font = cv2.FONT_HERSHEY_SIMPLEX
        font_scale = 1.0
        font_thickness = 2
        text_color = (255, 255, 255)  # Green color (in BGR format)
        text_position = (10, 60)  # Top-left corner position (x, y)

        # Write the text on the image
        cv2.putText(combined_img, text, text_position, font, font_scale, text_color, font_thickness)

        # Display or save the image with the frame number
        #cv2.imshow("Image with Frame Number", combined_img)
        #cv2.waitKey(0)
        #cv2.destroyAllWindows()
        #break
        all_frames.append(combined_img)

        video_writer.write(combined_img)
        frame_number += 1
        #cv2.waitKey(30)  # Display each frame for 30 milliseconds
        
        #cv2.imwrite(filename.replace(".jpg","_corrected.jpg"), undistorted_img)
        #cv2.waitKey(0)
        #break
    for i in all_frames:
        video_writer.write(i)
    for i in all_frames:
        video_writer.write(i)  

    video_writer.release()
    #cv2.destroyAllWindows()

def processImagesIntoPointCloud(images,mtx,dist,output_file):
    if os.path.exists(output_file):
        # Delete the file if it exists
        os.remove(output_file)
        print(f"Deleted existing {output_file}")

    z_value = 0

    for filename in images:
        v_width = 1280
        v_height = 1024
        small_img = cv2.imread(filename) # 900x900
        distorted_img = pad_image(small_img, 2560, 1440) # 2560x1440
        # Undistort the image
        undistorted_img = cv2.undistort(distorted_img, mtx, dist) # 2560x1440
        cropped_image = crop_image(undistorted_img, v_width,v_height)

        #cv2.imwrite(filename.replace(".jpg","_cropped_image.jpg"), cropped_image)

        red_threshold = 10

        # Find red pixels
        x_coords, y_coords = find_red_pixels(filename,cropped_image, red_threshold)
        # Create a Z-coordinate array of the same length as X and Y coordinates
        z_coords = np.zeros_like(x_coords)
        print(f"Number of red pixels: {len(x_coords)}")
        # Combine X, Y, and Z coordinates to form the 3D points
        data = list(zip(x_coords, y_coords, z_coords))

        axis_of_rotation = (670, 0, 0) # I recon the center of rotation is about 670 (should be 640)
        # Rotate the data around the Y-axis with translation
        rotated_data = rotate_data_around_y_with_translation(data, z_value*(360.0/len(images)), axis_of_rotation)
        rx_coords, ry_coords, rz_coords = zip(*rotated_data)
        # Write the coordinates to the .xyz file
        write_xyz_file(rx_coords, ry_coords,rz_coords,output_file)

        z_value += 1
        #return
        #cv2.destroyAllWindows()

    print("Load a ply point cloud, print it, and render it")
    pcd = o3d.io.read_point_cloud(output_file)
    print(pcd)
    print(np.asarray(pcd.points))

    datum_point_cloud = makeDatumAsPointCloud()

    # Define the translation vector to move the datum to the axis of rotation
    translation_vector = np.array([v_width/2, 0.0, 0.0])  # Adjust this as needed

    # Create a transformation matrix for translation
    translation_matrix = np.eye(4)
    translation_matrix[:3, 3] = translation_vector

    # Apply the translation to the entire point cloud
    datum_point_cloud = datum_point_cloud.transform(translation_matrix)

    combined_geometries = datum_point_cloud + pcd

    o3d.visualization.draw_geometries([combined_geometries],
                                    zoom=0.3412,
                                    front=[0.4257, -0.2125, -0.8795],
                                    lookat=[2.6172, 2.0475, 1.532],
                                    up=[-0.0694, -0.9768, 0.2024])

## Helpers for processing

def find_red_pixels(filename,image, threshold):
    # Convert BGR image to HSV
    hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

    # Define lower and upper threshold values for red (HSV color space)
    lower_red = np.array([00, 000, 240], dtype=np.uint8)
    upper_red = np.array([100, 255, 255], dtype=np.uint8)

    # Create a binary mask for red pixels
    red_mask = cv2.inRange(hsv_image, lower_red, upper_red)

    # Set the border regions to zero (black) in the mask
    top_border_size = 100
    bottom_border_size = 60
    left_border_size = 200
    right_border_size = 200
    red_mask[:top_border_size, :] = 0  # Top border
    red_mask[-bottom_border_size:, :] = 0  # Bottom border
    red_mask[:, :left_border_size] = 0  # Left border
    red_mask[:, -right_border_size:] = 0  # Right border
    # Create a white image of the same size as the original image
    white_image = np.ones_like(image) * 255
 
    # Apply the mask to the original image
    result = cv2.bitwise_and(image, image, mask=red_mask)
    #cv2.imshow("Mask applied {}".format(filename), result)
    #cv2.imshow("Original applied {}".format(filename), image)
    #cv2.imwrite(filename.replace(".jpg","_masked.jpg"), image)
    #cv2.waitKey(0)
    
    # Find the x, y coordinates of red pixels
    y_coords, x_coords = np.where(red_mask > 0)

    return x_coords, y_coords

def write_xyz_file(x_coords, y_coords, z_coords, output_file):
    with open(output_file, 'a') as f:
        for x, y, z in zip(x_coords, y_coords, z_coords):
            f.write(f"{x} {y} {z}\n")

def rotate_around_y_with_translation(point, angle, axis_of_rotation):
    # Extract the X, Y, and Z coordinates
    x, y, z = point

    # Extract the X, Y, and Z coordinates of the axis of rotation
    cx, cy, cz = axis_of_rotation

    # Translate the point and axis of rotation to the origin
    translated_x, translated_z = x - cx, z - cz

    # Convert the angle to radians
    theta = angle * np.pi / 180.0

    # Perform the rotation around the Y-axis
    cos_theta = np.cos(theta)
    sin_theta = np.sin(theta)
    new_translated_x = translated_x * cos_theta - translated_z * sin_theta
    new_translated_z = translated_x * sin_theta + translated_z * cos_theta

    # Translate the rotated point back to its original position
    new_x, new_z = new_translated_x + cx, new_translated_z + cz

    return new_x, y, new_z

def rotate_data_around_y_with_translation(data, angle, axis_of_rotation):
    rotated_data = []
    for point in data:
        rotated_point = rotate_around_y_with_translation(point, angle, axis_of_rotation)
        rotated_data.append(rotated_point)
    return rotated_data

def render_circle_in_frame(frame_width, frame_height, circle_radius):
    # Create a blank image frame filled with zeros (black color)
    image = np.zeros((frame_height, frame_width), dtype=np.uint8)

    # Calculate the center of the frame
    center_x = frame_width // 2
    center_y = frame_height // 2

    # Generate coordinates for the circle
    x_coords, y_coords = [], []
    for x in range(frame_width):
        for y in range(frame_height):
            #if (x - center_x) ** 2 + (y - center_y) ** 2 <= circle_radius ** 2:
            if (x - center_x) ** 2 + (y - center_y) ** 2 == circle_radius ** 2:
                x_coords.append(x)
                y_coords.append(y)
                # Set the pixel to a white color (255) for visualization purposes
                image[y, x] = 255

    return x_coords, y_coords, image

run = 75
dark_images = glob.glob('./Mk5/Code/15_Capture360_DarkOrLight/run_0{}/*.jpg'.format(run))
video_output_file = "./Mk5/Code/15_Capture360_DarkOrLight/run_0{}.mp4".format(run)  # Name of the output video file
s_video_output_file = "./Mk5/Code/15_Capture360_DarkOrLight/run_0{}_x_run_0{}.mp4".format(run,run+1)  # Name of the output video file
pc_output_file = "./Mk5/Code/15_Capture360_DarkOrLight/run_0{}_output.xyz".format(run)

print ("found {} dark images".format(len(dark_images)))

# Load the camera calibration data from the file
data = np.load("./Mk5/Code/13_Capture360_Crop/ov5640_camera_calibration_2560x1440_24.npz")
# Extract the camera matrix and distortion coefficients
mtx = data['mtx']
dist = data['dist']

makeVideo = False
makeSideBySideVideo = False
processFirstImageTest = True



if makeVideo:
    make_single_video(dark_images,mtx,dist,video_output_file)

if makeSideBySideVideo:
    light_images = glob.glob('./Mk5/Code/15_Capture360_DarkOrLight/run_0{}/*.jpg'.format(run+1))
    print ("found {} light images".format(len(light_images)))
    make_side_by_side_video(dark_images,light_images,mtx,dist,s_video_output_file)

if processFirstImageTest:
    processImagesIntoPointCloud(dark_images,mtx,dist,pc_output_file)
    
            