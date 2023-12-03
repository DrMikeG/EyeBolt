import open3d as o3d
import numpy as np
import matplotlib.pyplot as plt
from datum import makeDatumAsPointCloud

def custom_draw_geometry(pcd):
    vis = o3d.visualization.Visualizer()
    vis.create_window()  # Set the window size
    vis.add_geometry(pcd)
    vis.run()
    vis.destroy_window()

#output_file = "./Mk5/Code/20_100StepTurnsTakingPictures/run_037_output.xyz"
output_file = "./Mk5/Code/15_Capture360_DarkOrLight/run_077_output.xyz"

print("Load a ply point cloud, print it, and render it")
pcd = o3d.io.read_point_cloud(output_file)
print(pcd)
print(np.asarray(pcd.points))

datum_point_cloud = makeDatumAsPointCloud()

combined_geometries = datum_point_cloud + pcd

o3d.visualization.draw_geometries([combined_geometries],
                                width=1280,
                                height=720,
                                zoom=0.3412,
                                front=[0.4257, -0.2125, -0.8795],
                                lookat=[2.6172, 2.0475, 1.532],
                                up=[-0.0694, -0.9768, 0.2024])



# Create a sample point cloud
#pcd = o3d.geometry.PointCloud()
#pcd.points = o3d.utility.Vector3dVector([[0, 0, 0], [1, 0, 0], [0, 1, 0]])

# Call the custom draw function
#custom_draw_geometry(pcd)