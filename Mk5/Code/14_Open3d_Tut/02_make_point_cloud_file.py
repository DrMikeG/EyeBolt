import numpy as np
import cv2
import glob
import os
import open3d as o3d
from datum import makeDatumAsPointCloud

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



images = glob.glob('./Mk5/Code/13_Capture360_Crop/run_070/*.jpg')
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
    output_file = "./Mk5/Code/13_Capture360_Crop/run_070.mp4"  # Name of the output video file
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
output_file = "./Mk5/Code/14_Open3d_Tut/run_70_output.xyz"
if os.path.exists(output_file):
    # Delete the file if it exists
    os.remove(output_file)
    print(f"Deleted existing {output_file}")

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
        # Create a Z-coordinate array of the same length as X and Y coordinates
        z_coords = np.zeros_like(x_coords)
        print(f"Number of red pixels: {len(x_coords)}")
        # Combine X, Y, and Z coordinates to form the 3D points
        data = list(zip(x_coords, y_coords, z_coords))

        axis_of_rotation = ((v_width/2)+200, 0, 0)
        # Rotate the data around the Y-axis with translation
        rotated_data = rotate_data_around_y_with_translation(data, z_value*(360.0/len(images)), axis_of_rotation)
        rx_coords, ry_coords, rz_coords = zip(*rotated_data)
        # Write the coordinates to the .xyz file
        write_xyz_file(rx_coords, ry_coords,rz_coords,output_file)


        z_value += 1
        #break

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
            