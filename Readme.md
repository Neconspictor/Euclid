# Euclid 3D

![Euclid 3D in action](/readme-resources/Euclid-in-action.png)

Euclid is a render engine created for learning computer graphics algorithms. It is written in C++ and uses currently OpenGL 4.6 and GLSL for its render backend.


## Build Instructions

### 1. Requirements
* Python 3 and pip
* Graphics card supporting OpenGL 4.6
* Visual Studio 2019 x64 (other compilers not tested / 3rd party libraries not supported by dependency downloader)


### 2. Build Steps
* Execute dependency-downloader/install-python-deps.bat for inbstalling python dependencies for the dependency downloader
* Execute dependency-downloader/dependency-downloader.py and download all 3rd party dependencies and resources.
  The dependency downloader looks like that:
  ![Euclid 3D in action](/readme-resources/dependency-downloader-image.png)
* Execute createVS2019Build.bat for generating Visual Studio solution files in the 'build' subfolder 
* navigate to 'build', open the Euclid.sln and build the solution.   
  
### 3. Manual 3rd party library build steps (TODO)
* The Assimp 5.0.0 library is used but with a few changes. Use this fork please: https://github.com/Neconspictor/Euclid_Assimp
* Boost 1.67: atomic, boost, chrono, date_time, filesystem, locale, program_options, system, thread 
* GLFW 3.2.1