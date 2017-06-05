# Arcade-Game-Engine
A game engine specifically designed for making classic 80's-style arcade games. The work originated from the course **TDA572 Game Engine Architecture VT17** held at Chalmers University of Technology. This work has spiraled out into a 3D engine project since, for another course **DAT205 - Advanced Computer Graphics**, see 3D branch.

## Setup
To get the project up and running, you need to include the [SDL2](https://www.libsdl.org/download-2.0.php) and [SDL_image](https://www.libsdl.org/projects/SDL_image/) libraries into the project folder. Additionaly, [TinyXML-2](https://github.com/leethomason/tinyxml2) is used for reading XML files used in the audio synthesizer for the game engine.

### macOS
Download the development libraries for SDL2 and SDL_image for macOS, and place them in the path *Arcade Game Engine/external* relative the project path. In *external*, also create a folder called *tinyxml2* and put the files *tinyxml2.cpp* and *tinyxml2.h* in there from the TinyXML-2 project.

### Windows
Download the Visual Studio development libraries for SDL2 and SDL_image for Windows, and place them in the path *Arcade Game Engine/external* relative the project path. Extract all the .dll files from the respective *lib* paths of the libraries, and place them in the root of the project path. In *external*, also create a folder called *tinyxml2* and put the files *tinyxml2.cpp* and *tinyxml2.h* in there from the TinyXML-2 project.
