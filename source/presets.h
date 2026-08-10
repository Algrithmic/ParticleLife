/**
 * @file presets.h
 * @brief Built-in color palettes and attraction-matrix presets.
 *
 * Defines the selectable color palettes and the named attraction-matrix presets
 * offered in the GUI, along with their display names. Each table is indexed by its
 * matching enum, and the trailing *_COUNT enumerator gives the number of presets.
 */
#ifndef PRESETS_H
#define PRESETS_H

#include "particle.h"   // NUM_CHANNELS

/// Selectable color palette; also indexes color_presets[] and color_preset_names[].
typedef enum presets {
    RAINBOW,
    SUNSET,
    OCEAN,
    FOREST,
    PASTEL,
    NEON,
    AUTUMN,
    GRAYSCALE,
    CANDY,
    LAVA,
    ICE,
    JEWEL,
    VAPORWAVE,
    EARTH,
    COLOR_PRESET_COUNT
} color_preset_t;

#define COLORS_PER_PRESET   8

/// Human-readable names for each color palette, indexed by color_preset_t.
static char const * const color_preset_names[COLOR_PRESET_COUNT] = {
    "Rainbow",
    "Sunset",
    "Ocean",
    "Forest",
    "Pastel",
    "Neon",
    "Autumn",
    "Grayscale",
    "Candy",
    "Lava",
    "Ice",
    "Jewel",
    "Vaporwave",
    "Earth"
};

/// RGBA color values for each palette: [preset][class][channel], all in [0, 1].
static float const color_presets[COLOR_PRESET_COUNT][COLORS_PER_PRESET][NUM_CHANNELS] = {
    [RAINBOW] = {
        {1.000f, 0.000f, 0.000f, 1.0f},
        {1.000f, 0.498f, 0.000f, 1.0f},
        {1.000f, 1.000f, 0.000f, 1.0f},
        {0.000f, 1.000f, 0.000f, 1.0f},
        {0.000f, 0.500f, 1.000f, 1.0f},
        {0.294f, 0.000f, 0.510f, 1.0f},
        {0.560f, 0.000f, 1.000f, 1.0f},
        {1.000f, 0.412f, 0.706f, 1.0f}
    },
    [SUNSET] = {
        {0.984f, 0.686f, 0.404f, 1.0f},
        {0.976f, 0.522f, 0.325f, 1.0f},
        {0.937f, 0.400f, 0.318f, 1.0f},
        {0.847f, 0.290f, 0.353f, 1.0f},
        {0.686f, 0.220f, 0.416f, 1.0f},
        {0.478f, 0.184f, 0.427f, 1.0f},
        {0.290f, 0.153f, 0.365f, 1.0f},
        {0.145f, 0.106f, 0.259f, 1.0f}

    },
    [OCEAN] = {
        {0.827f, 0.941f, 0.949f, 1.0f},
        {0.643f, 0.867f, 0.898f, 1.0f},
        {0.435f, 0.769f, 0.827f, 1.0f},
        {0.263f, 0.647f, 0.741f, 1.0f},
        {0.145f, 0.502f, 0.616f, 1.0f},
        {0.086f, 0.365f, 0.478f, 1.0f},
        {0.043f, 0.235f, 0.353f, 1.0f},
        {0.020f, 0.129f, 0.235f, 1.0f}

    },
    [FOREST] = {
        {0.855f, 0.902f, 0.749f, 1.0f},
        {0.706f, 0.796f, 0.529f, 1.0f},
        {0.549f, 0.686f, 0.361f, 1.0f},
        {0.400f, 0.573f, 0.271f, 1.0f},
        {0.278f, 0.463f, 0.220f, 1.0f},
        {0.184f, 0.365f, 0.192f, 1.0f},
        {0.114f, 0.263f, 0.153f, 1.0f},
        {0.063f, 0.169f, 0.106f, 1.0f}

    },
    [PASTEL] = {
        {0.988f, 0.816f, 0.816f, 1.0f},
        {1.000f, 0.898f, 0.784f, 1.0f},
        {1.000f, 0.949f, 0.792f, 1.0f},
        {0.784f, 0.941f, 0.831f, 1.0f},
        {0.796f, 0.898f, 0.949f, 1.0f},
        {0.796f, 0.827f, 0.980f, 1.0f},
        {0.898f, 0.816f, 0.949f, 1.0f},
        {0.988f, 0.816f, 0.918f, 1.0f}

    },
    [NEON] = {
        {1.000f, 0.000f, 0.431f, 1.0f},
        {1.000f, 0.263f, 0.816f, 1.0f},
        {0.792f, 0.000f, 1.000f, 1.0f},
        {0.412f, 0.000f, 1.000f, 1.0f},
        {0.000f, 0.573f, 1.000f, 1.0f},
        {0.000f, 1.000f, 0.831f, 1.0f},
        {0.596f, 1.000f, 0.000f, 1.0f},
        {1.000f, 0.925f, 0.000f, 1.0f}

    },
    [AUTUMN] = {
        {0.647f, 0.165f, 0.165f, 1.0f},
        {0.804f, 0.333f, 0.129f, 1.0f},
        {0.855f, 0.475f, 0.129f, 1.0f},
        {0.894f, 0.635f, 0.161f, 1.0f},
        {0.741f, 0.529f, 0.184f, 1.0f},
        {0.545f, 0.365f, 0.169f, 1.0f},
        {0.361f, 0.239f, 0.129f, 1.0f},
        {0.204f, 0.145f, 0.098f, 1.0f}

    },
    [GRAYSCALE] = {
        {0.120f, 0.120f, 0.120f, 1.0f},
        {0.246f, 0.246f, 0.246f, 1.0f},
        {0.371f, 0.371f, 0.371f, 1.0f},
        {0.497f, 0.497f, 0.497f, 1.0f},
        {0.623f, 0.623f, 0.623f, 1.0f},
        {0.749f, 0.749f, 0.749f, 1.0f},
        {0.874f, 0.874f, 0.874f, 1.0f},
        {1.000f, 1.000f, 1.000f, 1.0f}
    },
    [CANDY] = {
        {1.000f, 0.412f, 0.706f, 1.0f},
        {1.000f, 0.600f, 0.800f, 1.0f},
        {0.988f, 0.451f, 0.451f, 1.0f},
        {1.000f, 0.714f, 0.400f, 1.0f},
        {1.000f, 0.925f, 0.451f, 1.0f},
        {0.616f, 0.878f, 0.518f, 1.0f},
        {0.545f, 0.800f, 0.941f, 1.0f},
        {0.780f, 0.549f, 0.925f, 1.0f}
    },
    [LAVA] = {
        {0.098f, 0.020f, 0.020f, 1.0f},
        {0.318f, 0.031f, 0.020f, 1.0f},
        {0.545f, 0.078f, 0.020f, 1.0f},
        {0.749f, 0.176f, 0.020f, 1.0f},
        {0.906f, 0.322f, 0.024f, 1.0f},
        {0.980f, 0.510f, 0.055f, 1.0f},
        {1.000f, 0.706f, 0.180f, 1.0f},
        {1.000f, 0.902f, 0.494f, 1.0f}
    },
    [ICE] = {
        {0.043f, 0.145f, 0.278f, 1.0f},
        {0.075f, 0.271f, 0.451f, 1.0f},
        {0.145f, 0.427f, 0.612f, 1.0f},
        {0.267f, 0.588f, 0.741f, 1.0f},
        {0.435f, 0.729f, 0.831f, 1.0f},
        {0.631f, 0.847f, 0.902f, 1.0f},
        {0.804f, 0.929f, 0.957f, 1.0f},
        {0.937f, 0.984f, 1.000f, 1.0f}
    },
    [JEWEL] = {
        {0.827f, 0.106f, 0.235f, 1.0f},
        {0.827f, 0.298f, 0.106f, 1.0f},
        {0.910f, 0.663f, 0.078f, 1.0f},
        {0.106f, 0.616f, 0.376f, 1.0f},
        {0.078f, 0.514f, 0.588f, 1.0f},
        {0.114f, 0.310f, 0.663f, 1.0f},
        {0.412f, 0.157f, 0.639f, 1.0f},
        {0.694f, 0.122f, 0.514f, 1.0f}
    },
    [VAPORWAVE] = {
        {1.000f, 0.443f, 0.831f, 1.0f},
        {0.949f, 0.545f, 0.910f, 1.0f},
        {0.729f, 0.529f, 0.961f, 1.0f},
        {0.529f, 0.596f, 0.980f, 1.0f},
        {0.416f, 0.784f, 0.965f, 1.0f},
        {0.408f, 0.945f, 0.925f, 1.0f},
        {0.647f, 0.980f, 0.820f, 1.0f},
        {0.988f, 0.850f, 0.949f, 1.0f}
    },
    [EARTH] = {
        {0.361f, 0.251f, 0.169f, 1.0f},
        {0.545f, 0.396f, 0.259f, 1.0f},
        {0.706f, 0.541f, 0.353f, 1.0f},
        {0.831f, 0.706f, 0.514f, 1.0f},
        {0.620f, 0.588f, 0.396f, 1.0f},
        {0.478f, 0.522f, 0.318f, 1.0f},
        {0.329f, 0.427f, 0.278f, 1.0f},
        {0.208f, 0.302f, 0.239f, 1.0f}
    }
};

/// Named attraction-matrix preset; indexes matrix_presets[] and matrix_preset_names[].
typedef enum matrix_presets {
    SNAKE,          // cyclic chase: each class hunts the next -> long chains
    PREDATOR_PREY,  // rock-paper-scissors cycle: chase prey, flee predator
    CELLS,          // strong self + directional cross links -> cell membranes
    CHECKERBOARD,   // parity-based attraction -> alternating segregation
    SYMMETRIC,      // like attracts like (distance based) -> layered clusters
    CLUMPS,         // strong self-attraction, mutual repulsion -> tight blobs
    ORBIT,          // attracted ahead, repelled behind -> rotating rings
    STAR,           // one universal attractor class -> core with orbiting shells
    WEB,            // classes pair off and bond -> connected lattice / network
    GAS,            // everything repels everything -> even dispersion
    CHAOS,          // fixed but irregular matrix -> unpredictable churn
    FACTIONS,       // two internally-bonded teams that repel each other -> segregation
    CRYSTAL,        // strong self + adjacent-class bonds -> connected lattice grains
    VORTEX,         // chase next, flee previous (strong) -> fast rotating swirls
    HIERARCHY,      // linear food chain: chase prey, flee predator -> traveling fronts
    NEBULA,         // sparse strong bonds among a mostly-neutral matrix -> wispy filaments
    RINGS,          // banded: attract near classes, repel far -> concentric shells
    MATRIX_PRESET_COUNT
} matrix_preset_t;

/// Human-readable names for each attraction-matrix preset, indexed by matrix_preset_t.
static char const * const matrix_preset_names[MATRIX_PRESET_COUNT] = {
    "Snake",
    "Predator-Prey",
    "Cells",
    "Checkerboard",
    "Symmetric",
    "Clumps",
    "Orbit",
    "Star",
    "Web",
    "Gas",
    "Chaos",
    "Factions",
    "Crystal",
    "Vortex",
    "Hierarchy",
    "Nebula",
    "Rings"
};

/// Attraction weights for each preset: a row-major MAX_NUM_CLASSES^2 matrix, values in [-1, 1].
static float const matrix_presets[MATRIX_PRESET_COUNT][MAX_NUM_CLASSES * MAX_NUM_CLASSES] = {
    [SNAKE] = {
         0.3f,  1.0f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,
        -0.2f,  0.3f,  1.0f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,
        -0.2f, -0.2f,  0.3f,  1.0f, -0.2f, -0.2f, -0.2f, -0.2f,
        -0.2f, -0.2f, -0.2f,  0.3f,  1.0f, -0.2f, -0.2f, -0.2f,
        -0.2f, -0.2f, -0.2f, -0.2f,  0.3f,  1.0f, -0.2f, -0.2f,
        -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,  0.3f,  1.0f, -0.2f,
        -0.2f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,  0.3f,  1.0f,
         1.0f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,  0.3f
    },
    [PREDATOR_PREY] = {
         0.2f, -0.8f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.8f,
         0.8f,  0.2f, -0.8f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.0f,  0.8f,  0.2f, -0.8f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.0f,  0.0f,  0.8f,  0.2f, -0.8f,  0.0f,  0.0f,  0.0f,
         0.0f,  0.0f,  0.0f,  0.8f,  0.2f, -0.8f,  0.0f,  0.0f,
         0.0f,  0.0f,  0.0f,  0.0f,  0.8f,  0.2f, -0.8f,  0.0f,
         0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.8f,  0.2f, -0.8f,
        -0.8f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.8f,  0.2f
    },
    [CELLS] = {
         0.8f,  0.3f, -0.1f, -0.1f, -0.1f, -0.1f, -0.1f, -0.4f,
        -0.4f,  0.8f,  0.3f, -0.1f, -0.1f, -0.1f, -0.1f, -0.1f,
        -0.1f, -0.4f,  0.8f,  0.3f, -0.1f, -0.1f, -0.1f, -0.1f,
        -0.1f, -0.1f, -0.4f,  0.8f,  0.3f, -0.1f, -0.1f, -0.1f,
        -0.1f, -0.1f, -0.1f, -0.4f,  0.8f,  0.3f, -0.1f, -0.1f,
        -0.1f, -0.1f, -0.1f, -0.1f, -0.4f,  0.8f,  0.3f, -0.1f,
        -0.1f, -0.1f, -0.1f, -0.1f, -0.1f, -0.4f,  0.8f,  0.3f,
         0.3f, -0.1f, -0.1f, -0.1f, -0.1f, -0.1f, -0.4f,  0.8f
    },
    [CHECKERBOARD] = {
         0.6f, -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,
        -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,  0.6f,
         0.6f, -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,
        -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,  0.6f,
         0.6f, -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,
        -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,  0.6f,
         0.6f, -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,
        -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,  0.6f, -0.6f,  0.6f
    },
    [SYMMETRIC] = {
         1.000f,  0.714f,  0.429f,  0.143f, -0.143f, -0.429f, -0.714f, -1.000f,
         0.714f,  1.000f,  0.714f,  0.429f,  0.143f, -0.143f, -0.429f, -0.714f,
         0.429f,  0.714f,  1.000f,  0.714f,  0.429f,  0.143f, -0.143f, -0.429f,
         0.143f,  0.429f,  0.714f,  1.000f,  0.714f,  0.429f,  0.143f, -0.143f,
        -0.143f,  0.143f,  0.429f,  0.714f,  1.000f,  0.714f,  0.429f,  0.143f,
        -0.429f, -0.143f,  0.143f,  0.429f,  0.714f,  1.000f,  0.714f,  0.429f,
        -0.714f, -0.429f, -0.143f,  0.143f,  0.429f,  0.714f,  1.000f,  0.714f,
        -1.000f, -0.714f, -0.429f, -0.143f,  0.143f,  0.429f,  0.714f,  1.000f
    },
    [CLUMPS] = {
         1.0f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f,
        -0.3f,  1.0f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f,
        -0.3f, -0.3f,  1.0f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f,
        -0.3f, -0.3f, -0.3f,  1.0f, -0.3f, -0.3f, -0.3f, -0.3f,
        -0.3f, -0.3f, -0.3f, -0.3f,  1.0f, -0.3f, -0.3f, -0.3f,
        -0.3f, -0.3f, -0.3f, -0.3f, -0.3f,  1.0f, -0.3f, -0.3f,
        -0.3f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f,  1.0f, -0.3f,
        -0.3f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f,  1.0f
    },
    [ORBIT] = {
         0.2f,  0.7f,  0.4f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
        -0.5f,  0.2f,  0.7f,  0.4f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.0f, -0.5f,  0.2f,  0.7f,  0.4f,  0.0f,  0.0f,  0.0f,
         0.0f,  0.0f, -0.5f,  0.2f,  0.7f,  0.4f,  0.0f,  0.0f,
         0.0f,  0.0f,  0.0f, -0.5f,  0.2f,  0.7f,  0.4f,  0.0f,
         0.0f,  0.0f,  0.0f,  0.0f, -0.5f,  0.2f,  0.7f,  0.4f,
         0.4f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,  0.2f,  0.7f,
         0.7f,  0.4f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,  0.2f
    },
    [STAR] = {
         0.5f,  0.1f,  0.1f,  0.1f,  0.1f,  0.1f,  0.1f,  0.1f,
         0.9f,  0.2f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f,
         0.9f, -0.3f,  0.2f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f,
         0.9f, -0.3f, -0.3f,  0.2f, -0.3f, -0.3f, -0.3f, -0.3f,
         0.9f, -0.3f, -0.3f, -0.3f,  0.2f, -0.3f, -0.3f, -0.3f,
         0.9f, -0.3f, -0.3f, -0.3f, -0.3f,  0.2f, -0.3f, -0.3f,
         0.9f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f,  0.2f, -0.3f,
         0.9f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f, -0.3f,  0.2f
    },
    [WEB] = {
         0.3f,  0.8f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,
         0.8f,  0.3f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,
        -0.2f, -0.2f,  0.3f,  0.8f, -0.2f, -0.2f, -0.2f, -0.2f,
        -0.2f, -0.2f,  0.8f,  0.3f, -0.2f, -0.2f, -0.2f, -0.2f,
        -0.2f, -0.2f, -0.2f, -0.2f,  0.3f,  0.8f, -0.2f, -0.2f,
        -0.2f, -0.2f, -0.2f, -0.2f,  0.8f,  0.3f, -0.2f, -0.2f,
        -0.2f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,  0.3f,  0.8f,
        -0.2f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,  0.8f,  0.3f
    },
    [GAS] = {
        -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f,
        -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f,
        -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f,
        -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f,
        -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f,
        -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f,
        -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f,
        -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f
    },
    [CHAOS] = {
         0.5f, -0.7f,  0.2f,  0.9f, -0.4f, -0.1f,  0.6f, -0.8f,
        -0.3f,  0.4f,  0.8f, -0.6f,  0.1f, -0.9f,  0.3f,  0.7f,
         0.7f, -0.2f, -0.5f,  0.3f,  0.9f, -0.4f, -0.8f,  0.1f,
        -0.6f,  0.9f, -0.1f,  0.5f, -0.7f,  0.2f,  0.4f, -0.3f,
         0.3f, -0.8f,  0.6f, -0.2f,  0.5f,  0.8f, -0.5f, -0.4f,
        -0.9f,  0.1f, -0.4f,  0.7f, -0.3f,  0.4f,  0.9f, -0.6f,
         0.4f,  0.6f, -0.7f, -0.1f,  0.8f, -0.5f,  0.2f,  0.5f,
        -0.2f, -0.5f,  0.9f,  0.4f, -0.6f,  0.7f, -0.3f,  0.6f
    },
    [FACTIONS] = {
         0.8f,  0.5f,  0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
         0.5f,  0.8f,  0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
         0.5f,  0.5f,  0.8f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,  0.8f, -0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f, -0.5f,  0.8f,  0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,  0.8f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,  0.5f,  0.8f,  0.5f,
        -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.8f
    },
    [CRYSTAL] = {
         1.0f,  0.4f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,  0.4f,
         0.4f,  1.0f,  0.4f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,
        -0.2f,  0.4f,  1.0f,  0.4f, -0.2f, -0.2f, -0.2f, -0.2f,
        -0.2f, -0.2f,  0.4f,  1.0f,  0.4f, -0.2f, -0.2f, -0.2f,
        -0.2f, -0.2f, -0.2f,  0.4f,  1.0f,  0.4f, -0.2f, -0.2f,
        -0.2f, -0.2f, -0.2f, -0.2f,  0.4f,  1.0f,  0.4f, -0.2f,
        -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,  0.4f,  1.0f,  0.4f,
         0.4f, -0.2f, -0.2f, -0.2f, -0.2f, -0.2f,  0.4f,  1.0f
    },
    [VORTEX] = {
         0.3f,  0.9f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.6f,
        -0.6f,  0.3f,  0.9f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.0f, -0.6f,  0.3f,  0.9f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.0f,  0.0f, -0.6f,  0.3f,  0.9f,  0.0f,  0.0f,  0.0f,
         0.0f,  0.0f,  0.0f, -0.6f,  0.3f,  0.9f,  0.0f,  0.0f,
         0.0f,  0.0f,  0.0f,  0.0f, -0.6f,  0.3f,  0.9f,  0.0f,
         0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.6f,  0.3f,  0.9f,
         0.9f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.6f,  0.3f
    },
    [HIERARCHY] = {
         0.1f,  0.8f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.8f,  0.1f,  0.8f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.0f, -0.8f,  0.1f,  0.8f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.0f,  0.0f, -0.8f,  0.1f,  0.8f,  0.0f,  0.0f,  0.0f,
         0.0f,  0.0f,  0.0f, -0.8f,  0.1f,  0.8f,  0.0f,  0.0f,
         0.0f,  0.0f,  0.0f,  0.0f, -0.8f,  0.1f,  0.8f,  0.0f,
         0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.8f,  0.1f,  0.8f,
         0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.8f,  0.1f
    },
    [NEBULA] = {
         0.2f,  0.0f,  0.7f,  0.0f,  0.0f, -0.5f,  0.0f,  0.0f,
         0.0f,  0.2f,  0.0f,  0.6f,  0.0f,  0.0f, -0.4f,  0.0f,
         0.7f,  0.0f,  0.2f,  0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
         0.0f,  0.6f,  0.0f,  0.2f,  0.0f,  0.0f,  0.0f, -0.5f,
         0.0f,  0.0f,  0.5f,  0.0f,  0.2f,  0.7f,  0.0f,  0.0f,
        -0.5f,  0.0f,  0.0f,  0.0f,  0.7f,  0.2f,  0.0f,  0.4f,
         0.0f, -0.4f,  0.0f,  0.0f,  0.0f,  0.0f,  0.2f,  0.6f,
         0.0f,  0.0f,  0.0f, -0.5f,  0.0f,  0.4f,  0.6f,  0.2f
    },
    [RINGS] = {
         0.8f,  0.3f, -0.1f, -0.4f, -0.4f, -0.4f, -0.4f, -0.4f,
         0.3f,  0.8f,  0.3f, -0.1f, -0.4f, -0.4f, -0.4f, -0.4f,
        -0.1f,  0.3f,  0.8f,  0.3f, -0.1f, -0.4f, -0.4f, -0.4f,
        -0.4f, -0.1f,  0.3f,  0.8f,  0.3f, -0.1f, -0.4f, -0.4f,
        -0.4f, -0.4f, -0.1f,  0.3f,  0.8f,  0.3f, -0.1f, -0.4f,
        -0.4f, -0.4f, -0.4f, -0.1f,  0.3f,  0.8f,  0.3f, -0.1f,
        -0.4f, -0.4f, -0.4f, -0.4f, -0.1f,  0.3f,  0.8f,  0.3f,
        -0.4f, -0.4f, -0.4f, -0.4f, -0.4f, -0.1f,  0.3f,  0.8f
    }
};


#endif // PRESETS_H