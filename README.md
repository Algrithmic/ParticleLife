# ParticleLife

<p align=center>  
  <img src="./assets/screenshot.png" width="40%" height="40%">  
</p>  

> [!NOTE]  
> This project is a work in progress and is not in it's final state!  

A ParticleLife Simulation written in C, using OpenGL and SDL3.
Particle Life is a particle simulation that produces emergent, lifelike behavior from a simple set of pairwise attraction rules.  

The interesting part is emergent complexity: the rules are entirely local and pairwise, but the global behavior — clusters forming, structures orbiting each other, species-like groupings — arises spontaneously from the random attraction matrix. Every run produces a different "ecosystem."  

## Dependencies

This software was developed in the Linux environment - specifically Ubuntu. The dependencies to run this software are as follows  

* gcc  
* make  
* SDL3 - windowing, context creation, and event handling. Vendored locally under `vendor/SDL3/`  
* GLAD - Loads OpenGL function pointers at runtime so the code can call OpenGL functions. vendored locally under `vendor/GLAD/`  
* cglm - C Math library used for the orthographic projection matrix. Linked as a system library `-lcglm`  
* OpenGL 4.3 - Required by the GPU driver for compute shaders used `#version 430 core`  

## Usage

### Build

To build ParticleLife run:

```bash
make all
```

This command will assemble and link all the source files and dependencies - creating the ParticleLife executable under the `build` directory.

## ToDo

* GUI Static Library for user interaction.  

## Contributions

**This project is not accepting contributions.**  
This is a personal project that I am maintaining on my own. Pull requests, merge requests, and issues will not be reviewed or addressed. Thank you for your interest.  
