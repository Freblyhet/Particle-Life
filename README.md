# Particle Life Simulation

## Overview
Particle Life is a physics-based simulation of emergent systems where particles interact through species–species force rules. Despite the simplicity of the interactions, the system produces flocking, orbiting, and self-organizing structures.

The project is implemented in **C++** with **OpenGL** for visualization, allowing you to watch the dynamics in real time.

## Features
- Particles interact according to user-defined force rules.
- Emergent behaviors like flocking, clustering, and orbiting.
- Real-time 2D visualization using OpenGL.
- Adjustable simulation parameters for experimentation.

## Requirements
- C++17 compatible compiler
- [GLFW](https://www.glfw.org/) (for window and input management)
- [GLAD](https://github.com/Dav1dde/glad) (for OpenGL function loading)
- OpenGL 3.3 or higher

## Setup / Build
1. Clone the repository:
   ```bash
   git clone https://github.com/Freblyhet/Particle-Life.git
   cd Particle-Life
   ```
2. Ensure GLFW and GLAD are installed and linked correctly.
3. Compile the code. Example using `g++`:
   ```bash
   g++ -std=c++17 src/*.cpp -o ParticleLife -lglfw -ldl -lGL
   ```
4. Run the simulation:
   ```bash
   ./ParticleLife
   ```

## Usage
- Adjust parameters in the `config.h` or main simulation file to change particle behavior.
- Observe how simple interaction rules lead to complex emergent structures.

## License
This project is open-source under the MIT License.  

---

> For more of my projects and source code, visit [my GitHub](https://github.com/Freblyhet).
