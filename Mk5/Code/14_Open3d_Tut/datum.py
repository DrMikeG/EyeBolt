import open3d as o3d
import numpy as np

def makeDatumAsPointCloud():
    # Create an array of points representing the datum
    start_points = np.array([
        [-1, 0, 0],  # X-axis start
        [0, -1, 0],  # Y-axis start
        [0, 0, -1]   # Z-axis start
    ])

    end_points = np.array([
        [1, 0, 0],   # X-axis end
        [0, 1, 0],   # Y-axis end
        [0, 0, 1]    # Z-axis end
    ])

    # Number of intermediate points between start and end points
    num_intermediate_points = 100

    # Calculate intermediate points
    intermediate_points = np.linspace(start_points, end_points, num=num_intermediate_points, axis=1)

    # Flatten the array of intermediate points
    intermediate_points = intermediate_points.reshape(-1, 3)

    # Combine the start, intermediate, and end points
    points = np.vstack([start_points, intermediate_points, end_points])

    # Create an Open3D point cloud from the points
    datum_point_cloud = o3d.geometry.PointCloud()
    datum_point_cloud.points = o3d.utility.Vector3dVector(points)

    return datum_point_cloud.scale(1000.0,center = datum_point_cloud.get_center())

def makeDatumAsPointCloud2():

    # Create an array of points representing the datum
    start_points = np.array([
        [-1, 0, 0],  # X-axis start
        [0, -1, 0],  # Y-axis start
        [0, 0, -1]   # Z-axis start
    ])

    end_points = np.array([
        [1, 0, 0],   # X-axis end
        [0, 1, 0],   # Y-axis end
        [0, 0, 1]    # Z-axis end
    ])

    # Number of intermediate points between start and end points
    num_intermediate_points = 100

    # Calculate intermediate points
    intermediate_points = np.linspace(start_points, end_points, num=num_intermediate_points, axis=1)

    # Flatten the array of intermediate points
    intermediate_points = intermediate_points.reshape(-1, 3)

    # Combine the start, intermediate, and end points
    points = np.vstack([start_points, intermediate_points, end_points])

    # Create an Open3D point cloud for the datum
    datum_point_cloud = o3d.geometry.PointCloud()
    datum_point_cloud.points = o3d.utility.Vector3dVector(points)
    return datum_point_cloud.scale(100.0,center = datum_point_cloud.get_center())