# ParticleLife

<p align=center>  
  <img src="./assets/logo.png" width="40%" height="40%">  
</p>  

> [!NOTE]  
> This project is a work in progress and is not in its final state!  

A ParticleLife Simulation written in C, using OpenGL and SDL3.
Particle Life is a particle simulation that produces emergent, lifelike behavior from a simple set of pairwise attraction rules.  

The interesting part is emergent complexity: the rules are entirely local and pairwise, but the global behavior — clusters forming, structures orbiting each other, species-like groupings — arises spontaneously from the random attraction matrix. Every run produces a different "ecosystem."  

## Controls

* **Left-click drag** — pan the camera around world space.
* **Scroll wheel** — zoom in/out, centered on the middle of the window.

The camera is independent of the simulated world: the world can be larger or smaller than the window, and panning/zooming never changes the simulation itself, only what part of it you're looking at.

## Features

The control panel (left side) exposes:

* **World Settings** — particle count, number of classes, per-class colors (with presets and randomize), and the world's boundary behavior:
  * **Infinite** — an unbounded plane; particles drift freely and are never wrapped.
  * **Toroidal** — a bounded, wrapping world whose Width/Height are independently adjustable; its extent is drawn as a white rectangle so you can see where particles wrap around the edges.
* **Attraction Matrix** — the per-class-pair attraction weights that drive the emergent behavior, editable individually or in groups (Ctrl+click to select multiple cells), with presets and randomize.
* **Physics**:
  * **Friction** — a velocity decay rate (0 = no damping, higher = more damping).
  * **Speed** — a multiplier applied to real elapsed time each frame, so the simulation runs at a consistent speed regardless of framerate or machine.
  * **Max Radius** — the maximum distance over which particles interact.

## System Requirements

The simulation's physics run as a brute-force, pairwise compute shader (every particle checks every other particle each frame, not a spatial partitioning scheme), so the GPU is the limiting factor at high settings, not the CPU.

Requirements below are sized for running at the maximum settings exposed in the GUI: **35,000 particles**, **8 classes**, world up to **3000×3000**, and **300** max interaction radius — at that count the compute shader is evaluating on the order of 1.2 billion particle-pair checks per frame.

* **OS** — Linux (developed on Ubuntu) or Windows 10/11 via MSYS2/MinGW-w64.
* **GPU** — a dedicated/discrete GPU supporting **OpenGL 4.3 core profile** (required for compute shaders) with up-to-date drivers. Integrated graphics may struggle to hold an interactive framerate at 35,000 particles; drop the particle count if you're seeing stutter.
* **CPU / RAM** — not a bottleneck at any setting; the per-frame work is almost entirely on the GPU.

Everything below the maximum is proportionally lighter — a few thousand particles runs comfortably on integrated graphics.

## Dependencies

This software was developed in the Linux environment - specifically Ubuntu, but also builds and runs on Windows via MSYS2/MinGW-w64. The dependencies to run this software are as follows  

* gcc  
* make  
* SDL3 - windowing, context creation, and event handling. Vendored locally under `vendor/SDL3/`  
* GLAD - Loads OpenGL function pointers at runtime so the code can call OpenGL functions. Vendored locally under `vendor/GLAD/`  
* Nuklear - Single-header immediate-mode GUI used for user interaction. Vendored locally under `vendor/Nuklear/`  
* cglm - C Math library used for the orthographic projection matrix. Linked as a system library `-lcglm`  
* OpenGL 4.3 - Required for the compute shaders (`#version 430 core`) that run the particle physics  

The vendored libraries (SDL3, GLAD, Nuklear) require no installation. The only dependency you need to install yourself is cglm:  

```bash
sudo apt install libcglm-dev
```

## Usage

### Build on Linux

This software was developed on Ubuntu; other distros should work as long as the [dependencies](#dependencies) are available.

#### 1. Install packages

```bash
sudo apt install gcc make libcglm-dev
```

#### 2. Build and run

From the project root:

```bash
make all
./build/ParticleLife
```

This assembles and links all the source files and dependencies, creating the `ParticleLife` executable under the `build` directory.

### Building on Windows

The source is portable C/SDL3/OpenGL with no Linux-specific code, so it builds with the same Makefile under [MSYS2](https://www.msys2.org/)'s MinGW-w64 toolchain. Native MSVC/CMake is not set up - use the MSYS2 **MINGW64** shell for all steps below.

#### 1. Install MSYS2 and packages

Install MSYS2 from https://www.msys2.org/, then open the **MSYS2 MINGW64** shell (not the plain MSYS2 or UCRT64 shell) and run:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-cglm
```

`make` is installed as `mingw32-make`; either alias it to `make` or invoke it directly (substitute `mingw32-make` for `make` in the commands below).

#### 2. Build and run

From the MINGW64 shell, in the project root:

```bash
make all
./build/ParticleLife.exe
```

The Makefile detects Windows automatically (no rpath support there) and copies `SDL3.dll` next to the executable after linking, so `ParticleLife.exe` finds it without needing it on `PATH`.

### Documentation

The source is annotated with Doxygen comments. To generate browsable HTML documentation (requires `doxygen` installed via `sudo apt install doxygen`):

```bash
make docs
```

The generated docs are written to `docs/html/index.html`.
