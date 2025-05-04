# MIMesh

**MIMesh** (Multi-resolution Interpolation Mesh) is a software tool for performing 3D morphing between two semi-regular meshes. It demonstrates the method proposed in our Pacific Graphics 2001 paper:

- Takashi Michikawa, Takashi Kanai, Masahiro Fujita, Hiroaki Chiyokura:
  *"Multiresolution Interpolation Meshes"*,
  Proceedings of the 9th Pacific Graphics International Conference (Pacific Graphics 2001), pp. 60–69, October 2001.

Note that this software is for demonstration purposes only and does not include an implementation of the method.

The software was originally developed in 2001 by [Takashi Michikawa](https://github.com/tmichi) and [Masahiro Fujita](https://github.com/syoyo), and was updated in 2025 to support building and running on Windows (Visual Studio), macOS, and Linux (Ubuntu).

## Getting Started

To compile the software, you need to have the following dependencies installed: GLFW3, and GLEW.

After installing these dependencies, run the following commands:

```bash
git clone https://github.com/kanait/mimesh.git
cd mimesh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cd ..
./build/mimesh
```

### Notes

- On **Windows**, you can install dependencies using [vcpkg](https://vcpkg.io/en/) and build the project using CMake with Visual Studio as the generator.

- On **macOS**, you can install dependencies using [Homebrew](https://brew.sh/):

  ```bash
  brew install glew glfw
  ```

- On **Ubuntu**, install dependencies via apt:

  ```bash
  sudo apt update
  sudo apt install build-essential cmake libglew-dev libglfw3-dev
  ```

## Usage

When the application starts, a display window will appear.
Right-click inside the window to open the context menu, which provides access to the following operations:

- Toggle normal mapping (on/off)
- Change the resolution of the normal map
- Start/stop morphing
- Switch between linear and Bézier interpolation

## License

This software is licensed under the MIT License.
See the [LICENSE](LICENSE) file for details.