# Computer Graphics project: 3d Space Partitioning

This project is meant to build a visualization tool for space partitioning algorithms. Originally, this project was meant to visualize both the octree and kd-tree algorithms, but due to circumstances the scope got reduced to only the octree. This does have the advantage that the whole implementation can be focused on this algorithm, while still trying to think of solutions on how the tool could be expanded to also visualize the kd-tree and possibly a bsp-tree.

## Installation and use

This project contains a node frontend and c++ backend, so you are assumed to have node installed and the means to build a cpp project using CMake. The frontend will automatically install the needed packages before launching, but the backend does not automatically build every time. You are expected to build this using the provided `CMakeLists.txt` file.

In case you are using linux, the `start.sh` script can be used to start both servers. In windows, you have to start both seperately.

## General notes

- Largest pcd test file is ~200MB and contains ~200 million points
- Three.JS has BoxGeometry, so a voxel just needs a coordinate and size

Below is a chart of the general pipeline of the application, where the boxes present **C++ code in the backend**, and the circle presents **javascript code in the frontend**. I opted for this division to get a good balance between performance when it's most needed (constructing and traversing the tree), and ease of visualization (Three.js instead of glu, glut, linking libraries, ...).

![flow](flow.png)

## Algorithm - octree

- Define an octant either by
  - Center and vector to a corner: [reference](https://github.com/brandonpelfrey/SimpleOctree/blob/master/Octree.h) (preferred)
  - Min and max corner: possible, but not practical
- Stop conditions:
  - **Predefined resolution (preferred)**: no control over file size of the resulting tree, but will give the best results for visualization; also feels more intuitively useful for a tool like this one
  - **Maximum height**: will give the most consistent results in file size, however the results of the visualization can greatly differ
- Start by center meaning the pointcloud (each point - average) => avoid pointcloud being off-centered
- Root could be defined with center (0, 0, 0) and half distance being the smallest power of 2 greater than the largest value on any axis (makes division by 2 cleaner)

```c++
constructTree(string pcdFilename): Octree
```

```c++
getCubes(int height): Cube[]
```

## Rendering

### Abstract idea

The rendering in this project is purely defined by cubes; meaning that the renderer should just be able to take a list of tuples containing a location and a size.

> In case the renderer should also be able to visualize a kd-tree, this could be generalized to a location with a vector to a corner, defining a cuboid. This would allow the renderer to be flexible enough to render both an octree and kd-tree.
