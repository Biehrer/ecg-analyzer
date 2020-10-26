
# ecg-analyzer
C++ libs for visualization and algorithmic analyzation of ecg signals.

customable, hardware accelerated realtime visualization of ecg signals in sweep charts on the gpu via OpenGL in C++

Supports reading of Phsyionet MIT-BIH-Database records files via the wfdb library and supports real-time QRS-Detection with the simple Pan-Topkins algorithm, implemented like described in the original paper

![bb](https://user-images.githubusercontent.com/44288843/82672250-67b55b00-9c40-11ea-8014-09ff70e12b38.png)

![qrs detector](https://user-images.githubusercontent.com/44288843/96221845-861f4880-0f8b-11eb-818a-eee13710c80e.png)

https://github.com/Biehrer/ecg-analyzer
## Get started

required dependencies:
 (Conan package manager is supported)
- Qt5 (Core/Widgets/GUI module)
- Freetype II for OGL glyph rendering
- WFDB C-Library (& Curl) for loading of MIT-BIH-Database records
- KFR library for DSP (Filtering)
- CPPUnit for unit testing

! You can install Freetype and KFR with the conan package manager(use the conanlists.txt inside the project directory)
### How 2 build:

- clone the repository: "git clone (*link_2_git_repo)"
- build the project using CMAKE 
- CMake asks for paths to Qt5Core/Qt5Widgets/Qt5Gui, Freetype (EXPLICIT! READ DOWN BELOW), KFR and CPPUnit(when tests are enabled)
To install Freetype correctly, download and build the library and go to the CMakeLists.txt file of this project.
Inside the CMakeLists.txt file, adapt the environment variable of the freetype path to the directory Freetype was installed on you machine.
- Press configure
- Open the .sln file, set the project 'signalanalyzer' (with main.cpp) as startup project and run the project

### How 2 run:
Some steps are necessary because of the projects early stage:

- Adapt hardcoded file paths (e.g strg-f search for C:/ inside the code and adapt it to you machine; should only be inside ogl_plot_renderer_widget.h/cpp to load the shaders from disk)

- Go to includes/CMakeList.txt and change the filepath in the following line to the location where your Freetype 2 build directy is located:
set(ENV{FREETYPE_DIR} "C:/Development/libs/freetype-build/")

See main.cpp for entry point of the program

Feel free to message me, if you would like to contribute to the project or sub-projects (e.g data visualization with OpenGL, implementation of algorithms)
