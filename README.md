# ecg-analyzer
Framework for visualization and algorithmic analyzation of ecg (electrocardiography) signals.

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
- Open the .sln file, set the project as startup project and run the project

### How 2 run:

Make sure the shaders (files inside the /Resources directory) are inside the BUILD directory you created with CMake.
The expected path to this directory is printed to the console. Make sure the files are there.
