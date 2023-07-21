import open3d as o3d
import numpy as np
from datum import makeDatumAsPointCloud

output_file = "./Mk5/Code/14_Open3d_Tut/run_71_output.xyz"

print("Load a ply point cloud, print it, and render it")
pcd = o3d.io.read_point_cloud(output_file)
print(pcd)
print(np.asarray(pcd.points))

datum_point_cloud = makeDatumAsPointCloud()

combined_geometries = datum_point_cloud + pcd

o3d.visualization.draw_geometries([combined_geometries],
                                zoom=0.3412,
                                front=[0.4257, -0.2125, -0.8795],
                                lookat=[2.6172, 2.0475, 1.532],
                                up=[-0.0694, -0.9768, 0.2024])