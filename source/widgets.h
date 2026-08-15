/**
 * @file widgets.h
 * @brief Custom Nuklear button styles used by the GUI.
 *
 * Declares helpers that build derived nk_style_button values for the control
 * panel: flat circular color swatches and attraction-factor cells whose fill
 * color encodes a weight.
 */
#ifndef CUSTOM_WIDGETS_H
#define CUSTOM_WIDGETS_H

#include "gui.h"
#include "presets.h"
#include "particle.h"

#define DEFAULT_PANEL_WIDTH     325
#define PANEL_CONTENT_RIGHT_PADDING 20
#define DEFAULT_WIDGET_HEIGHT   35
#define MAX_NUM_COUNT           8
#define EMPTY_STRING            ""
#define COLOR_RANGE             255.0f
#define TEXT_MAX_SIZE           256

/// Build a rounded, solid-color button style (used for circular color swatches).
struct nk_style_button circular_button(struct nk_context *context, struct nk_color color);

/// Build a rounded button style with a hue that cycles on every call (filled or bordered).
struct nk_style_button rainbow_button(struct nk_context *context, bool fill);

/// Build a button style whose fill color encodes an attraction weight in [-1, 1].
struct nk_style_button attraction_factor_button(struct nk_context *context, float *value);

/// Build a labeled row combining an integer slider with a typeable, self-syncing textbox.
bool uint_variable_slider(struct nk_context *context, char const * const label, uint32_t min, uint32_t *buffer, uint32_t max, char *text_buffer);

/// Build a labeled row combining a float slider with a typeable, self-syncing textbox.
bool float_variable_slider(struct nk_context *context, char const *label, float min, float *buffer, float max, float step, char *text_buffer);

/// Build an RGBA color-picker panel: an HSV wheel plus per-channel (0-255) int fields.
void extended_color_picker(struct nk_context *context, float color[NUM_CHANNELS]);

/// Attraction matrix selection state, updated by create_grid() and consumed by cell_editor().
typedef struct active_cell {
    bool    selection[MAX_NUM_CLASSES * MAX_NUM_CLASSES]; ///< Per-cell selected flag, indexed like the matrix (row-1)*MAX_NUM_CLASSES + (col-1).
    uint8_t rowidx;    ///< Row (1-based, header row is 0) of the most recently clicked cell.
    uint8_t colidx;    ///< Column (1-based, header column is 0) of the most recently clicked cell.
    bool    universal; ///< True when the "select all" corner cell is active; overrides selection[].
} active_cell_t;

// Attraction Matrix Section
/// Build the interactive attraction matrix grid; updates output with the current click/selection.
void create_grid(struct nk_context *context, float *values, float (*palette)[NUM_CHANNELS], uint8_t const dimensions, active_cell_t *output);

/// Build the value editor (swatches + slider/textbox) for the cells cell_data selects.
bool cell_editor(struct nk_context *context, attraction_t *attraction, active_cell_t cell_data, struct nk_style_button active, struct nk_style_button affected);

/// Build the attraction-matrix preset combobox and randomize button.
bool grid_preset_randomize(struct nk_context *context, attraction_t *attraction);

#endif