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

/// Build a rounded, solid-color button style (used for circular color swatches).
struct nk_style_button circular_button(struct nk_context *context, struct nk_color color);

/// Build a button style whose fill color encodes an attraction weight in [-1, 1].
struct nk_style_button attraction_factor_button(struct nk_context *context, float *value);

#endif