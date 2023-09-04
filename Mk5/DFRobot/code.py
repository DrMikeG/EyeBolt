import numpy as np
import matplotlib.pyplot as plt
import os

# Define the rainbow colors
rainbow_colors = [
    (255, 0, 0),      # Red
    (255, 127, 0),    # Orange
    (255, 255, 0),    # Yellow
    (0, 255, 0),      # Green
    (0, 0, 255),      # Blue
    (75, 0, 130),     # Indigo
    (148, 0, 211)     # Violet
]

# Generate 100 evenly spaced color values in the rainbow spectrum
num_colors = 100
colors = []

for i in range(num_colors):
    # Calculate the color as an interpolation between rainbow colors
    start_index = i * (len(rainbow_colors) - 1) // (num_colors - 1)
    end_index = start_index + 1
    if end_index >= len(rainbow_colors):
        end_index = 0
    fraction = (i - start_index * (num_colors - 1) // len(rainbow_colors)) / ((num_colors - 1) // len(rainbow_colors))
    
    start_color = rainbow_colors[start_index]
    end_color = rainbow_colors[end_index]
    
    r = int(np.interp(fraction, [0, 1], [start_color[0], end_color[0]]))
    g = int(np.interp(fraction, [0, 1], [start_color[1], end_color[1]]))
    b = int(np.interp(fraction, [0, 1], [start_color[2], end_color[2]]))
    
    # Append the RGB tuple to the list of colors
    colors.append((r, g, b))


# Define the parameters for the image and PDF
image_width_mm = 377  # Width of the image in mm
image_height_mm = 8  # Height of the image in mm
dpi = 300  # Dots per inch (resolution of the image)

# Calculate the image size in pixels
image_width_px = int(image_width_mm * dpi / 25.4)
image_height_px = int(image_height_mm * dpi / 25.4)

# Create a NumPy array to represent the image
image = np.ones((image_height_px, image_width_px, 3), dtype=np.uint8) * 255  # Initialize with white background

for x in range(100):
    
    x_offset = x * image_width_px // 100

    # rectangle background color
    width = (image_width_px // 100)  # Width of the rectangle
    height = image_height_px  # Height of the rectangle
    color = (200, 200, 200)  # RGB color (blue)
    # Draw the colored rectangle by setting pixel values
    image[1:height-1, x_offset:x_offset + width] = colors[x]
    h2 = height //4
    h3 = h2 * 3
    image[h2:h3, x_offset:x_offset + width] = (254,254,254)

    # Start vertical line    
    image[:, x_offset] = [0, 0, 0]  # Set grid lines to black

    # text
    cell_x_center = x_offset + (image_width_px // 200)
    cell_label = str(x).zfill(2)
    plt.text(cell_x_center, image_height_px // 2, cell_label, color='black', rotation=90,ha='center', va='center',fontsize=7)

image[:, image_width_px-2] = [0, 0, 0]  # Set grid lines to black
image[1, :] = [0, 0, 0]  # Set grid lines to black
image[image_height_px-1, :] = [0, 0, 0]  # Set grid lines to black

# Add numbers or labels to the grid
#for x in range(0, image_width_px, int(grid_spacing_mm * dpi / 25.4)):
#    for y in range(0, image_height_px, int(grid_spacing_mm * dpi / 25.4)):
#        number = f"{x//int(grid_spacing_mm * dpi / 25.4)},{y//int(grid_spacing_mm * dpi / 25.4)}"
#        plt.text(x + 5, y + 5, number, color='black')

# Display a preview of the image in a window on the screen
plt.imshow(image)
plt.axis('off')  # Turn off axis labels
plt.show()

# Get the directory where the source file is located
source_dir = os.path.dirname(os.path.abspath(__file__))
# Define the PDF file path based on the source file's directory
pdf_filename = os.path.join(source_dir, "grid_pattern.pdf")
# Create a PDF file and save the image to it

#fig = plt.figure(figsize=(image_width_mm, image_height_mm), dpi=dpi)
#fig.savefig(pdf_filename, format='pdf')

#print(f"Image saved as {pdf_filename}")
