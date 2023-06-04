# pip install opencv-python 
# pip install numpy

import cv2
import numpy as np

def convolution(image_path):
    # Load the image using OpenCV
    image = cv2.imread(image_path)

    # Perform your convolution operations on the image
    # ...

    # Create a new image to store the result
    result = np.zeros_like(image)

    # Iterate over each pixel in the image using nested loops
    for i in range(image.shape[0]):
        for j in range(image.shape[1]):
            # Perform convolution operation on the pixel
            # ...

            # Update the corresponding pixel in the result image
            result[i, j] = processed_pixel_value

    # Save the result image as a new file
    output_path = "./Mk5/FirstLight/2023-05-01_20-47-24_file_output.png"
    cv2.imwrite(output_path, result)

    print("Convolution completed and saved as", output_path)

# Call the convolution function with the image file path
image_path = "./Mk5/FirstLight/2023-05-01_20-47-24_file.jpeg"
convolution(image_path)
