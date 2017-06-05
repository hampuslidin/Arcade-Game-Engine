# RAW Engine
The *Render-A-World Engine* is a 3D game engine written in C++. The engine uses class inheritence to allow for game-specific behavior to easily be integrated into the engine pipeline. It first started out as a 2D engine for the course **TDA572 - Game Engine Architecture**, and then transformed into a 3D engine for another course **DAT205 - Advanced Computer Graphics**; both courses were held at *Chalmers University of Technology*.

## Setup
Just run the included CMake script (preferably in a separate *build* directory) and it will download all dependencies automatically. 

### macOS
To generate an Xcode project file, open up the Terminal and run the following:

    $ cd /path/to/project
    $ mkdir build
    $ cd build
    $ cmake -G Xcode ..

### Windows
To generate a Visual Studio 15 solution file, open up the Command Prompt and run the following:

    > cd /path/to/project
    > mkdir build
    > cd build
    > cmake -G "Visual Studio 15" -A Win64 ..
