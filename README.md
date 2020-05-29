
# ecg-analyzer
C++ libs for visualization and algorithmic analyzation of ecg signals.

customable, hardware accelerated realtime visualization of ecg signals in sweep charts on the gpu via OpenGL in C++

![bb](https://user-images.githubusercontent.com/44288843/82672250-67b55b00-9c40-11ea-8014-09ff70e12b38.png)

https://github.com/Biehrer/ecg-analyzer
## Get started

required dependencies:
- Qt5 (Core/Widgets/GUI module)
- Freetype II
- WFDB C-Library (& Curl)

### How 2 build:

- clone the repository: "git clone (*link_2_git_repo)"
- build the project using CMAKE 
- CMake asks for paths to Qt5Core/Qt5Widgets/Qt5Gui and Freetype (EXPLICIT! READ DOWN BELOW)
To install Freetype correctly, download and build the library and go to the CMakeLists.txt file of this project.
Inside the CMakeLists.txt file, adapt the environment variable of the freetype path to the directory Freetype was installed on you machine.
- Press configure
- Open the .sln file, set the project 'signalanalyzer' (with main.cpp) as startup project and run the project

### How 2 run:
Some steps are necessary because of the projects early stage:

- Adapt hardcoded file paths (e.g strg-f search for C:/ inside the code and adapt it to you machine; should only be inside ogl_plot_renderer_widget.h/cpp to load the shaders from disk)

- Go to includes/CMakeList.txt and change the filepath in the following line to the location where your Freetype 2 build directy is located:
set(ENV{FREETYPE_DIR} "C:/Development/libs/freetype-build/")

See main.cpp for example usage of the visualization module
