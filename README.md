# Terrain Generation C++

An [OpenGL]() voxel-based terrain generation application

_This project only targets Windows_

## Installation

_Currently, the project can only be built using the provided .sln file located in the project root directory_

* Clone repository:
    * ~~~
      git clone https://github.com/DeTosis/terrain-generation-cpp
      ~~~

* Using Visual Studio open `.sln` file in project root folder
    * Build application

* Run generated `.exe` file located in `./_build` folder

## Preview

<img width="1282" height="720" alt="image_2025-08-16_04-29-55" src="https://github.com/user-attachments/assets/2aaa8883-61ef-422d-a9d6-dfbc8433fb3d" />

## Features

* Invisible voxel's face culling
* Multi thread two-step chunk based world generation
* First-person camera controls
* Dynamic Vertex/Index buffer sizing
    * Free list based memory management
* Dynamic chunk allocation and deallocation
* Statistic display
* Customisable render distance 


## Dependencies
- [OpenGL](https://www.opengl.org/)  
- [GLEW](https://glew.sourceforge.net/)  
- [GLAD](https://www.glfw.org/)  
- [glm](https://github.com/g-truc/glm)  
- [ImGui](https://github.com/ocornut/imgui)  
- [FastNoiseLite](https://github.com/Auburn/FastNoiseLite)
