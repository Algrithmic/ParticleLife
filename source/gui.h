/**
 * @file gui.h
 * @brief Single configuration point for Nuklear and the GUI module API.
 *
 * Defines the Nuklear feature flags and includes the library and its SDL3/GL3
 * backend, so every translation unit shares one consistent configuration. The
 * matching *_IMPLEMENTATION macros are defined only in gui.c.
 */
#ifndef GUI_H
#define GUI_H

// Nuklear feature configuration — must precede the nuklear.h include below.
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#define MAX_VERTEX_MEMORY 512 * 1024    ///< Vertex buffer size for nk_sdl_render().
#define MAX_ELEMENT_MEMORY 128 * 1024   ///< Element (index) buffer size for nk_sdl_render().

#include "nuklear.h"
#include "nuklear_sdl3_gl3.h"

typedef struct application application_t;

// init_gui : creates the Nuklear context and bakes the default font
int init_gui(application_t *application);

// destroy_gui : tears down the Nuklear context and GPU resources
int destroy_gui(void);

// update_gui : builds the GUI layout for the current frame
int update_gui(application_t *application);

#endif