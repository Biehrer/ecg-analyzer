
# ecg-analyzer
C++ libs for visualization and algorithmic analyzation of ecg signals.

customable, hardware accelerated realtime visualization of ecg signals in sweep charts on the gpu via OpenGL in C++

![visualization_example](https://user-images.githubusercontent.com/44288843/81840727-b16bba80-9549-11ea-8863-91453e66e537.jpg)

https://github.com/Biehrer/ecg-analyzer
## Get started

required dependencies:
- Qt5 (Core/Widgets/GUI module)
- Freetype II

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

