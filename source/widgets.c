/**
 * @file widgets.c
 * @brief Custom Nuklear button styles for the control panel.
 *
 * Builds derived nk_style_button values from the context's base button style:
 * flat circular color swatches for class colors, and attraction-factor cells
 * whose fill color is interpolated from a weight in [-1, 1].
 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "widgets.h"

#define DEFAULT_ROUNDING            13.5f

/**
 * circular_button
 *
 * @brief Builds a flat, heavily rounded button style filled with a solid color.
 *
 * Copies the context's base button style, sets all three visual states (normal,
 * hover, active) to the same color so the swatch does not change on interaction,
 * and applies a large corner rounding to make a small square button read as a
 * circle. Used for the per-class color swatches and matrix headers.
 *
 * @param context  The Nuklear context whose base button style is copied.
 * @param color    The solid fill color for the button.
 * @return         A configured nk_style_button to pass to nk_button_*_styled().
 */
struct nk_style_button circular_button(struct nk_context *context, struct nk_color color) {
    struct nk_style_button button = context->style.button;
    
    // background
    button.normal = (struct nk_style_item) { .type = NK_STYLE_ITEM_COLOR, .data.color = color };
    button.hover  = (struct nk_style_item) { .type = NK_STYLE_ITEM_COLOR, .data.color = color };
    button.active = (struct nk_style_item) { .type = NK_STYLE_ITEM_COLOR, .data.color = color };

    // Text - Shouldn't be any text for circular buttons

    // Properties
    button.rounding = DEFAULT_ROUNDING;
    button.padding = (struct nk_vec2) { .x = 2.0f, .y = 0.0f };

    return button;
}

static struct nk_color hsv_to_rgb(float hue, float saturation, float value) {
    float red        = 0.0f;
    float green      = 0.0f;
    float blue       = 0.0f;

    float chroma = value * saturation;
    float interpol = chroma * (1.0f - fabsf(fmodf(hue / 60.0f, 2.0f) - 1.0f));
    float brightness = value - chroma;

    if (hue < 60.0f) {
        red = chroma; 
        green = interpol; 
        blue = 0;
    }
    else if (hue < 120.0f) {
        red = interpol; 
        green = chroma; 
        blue = 0;
    }
    else if (hue < 180.0f) {
        red = 0; 
        green = chroma; 
        blue = interpol;
    }
    else if (hue < 240.0f) {
        red = 0; 
        green = interpol; 
        blue = chroma;
    }
    else if (hue < 300.0f) {
        red = interpol; 
        green = 0; 
        blue = chroma;
    }
    else {
        red = chroma; 
        green = 0; 
        blue = interpol;
    }

    hue += 0.1f;
    if (hue >= 360.0f) hue = 0.0f;

    return (struct nk_color) {
        ( (red + brightness) * 255.0f),
        ( (green + brightness) * 255.0f),
        ( (blue + brightness) * 255.0f),
        255
    };
}

struct nk_style_button rainbow_button(struct nk_context *context) {
    static float hue = 0.0f;
    struct nk_color color = hsv_to_rgb(hue, 1.0f, 1.0f);

    hue += 0.1f;
    if (hue >= 360.0f) hue = 0.0f;

    return circular_button(context, color);
}

/**
 * attraction_color
 *
 * @brief Maps an attraction weight to a color along a red-neutral-blue gradient.
 *
 * Clamps the weight to [-1, 1], then linearly interpolates from a neutral gray at
 * 0 toward blue for positive (attractive) weights or red for negative (repulsive)
 * weights, with the magnitude controlling how far toward the target color.
 *
 * @param value  The attraction weight; clamped to [-1, 1].
 * @return       The interpolated color representing that weight.
 *
 * @note This is a static internal helper for attraction_factor_button().
 */
static struct nk_color attraction_color(float value) {
    // clamp the attraction factor within range [-1, 1]
    if (value >  1.0f)  value =  1.0f;
    if (value < -1.0f)  value = -1.0f;

    // attraction color representation
    struct nk_color const negative = (struct nk_color) { 255,  25,  25, 255};
    struct nk_color const neutral  = (struct nk_color) {  68,  68,  75, 255};
    struct nk_color const positive = (struct nk_color) {  50, 132, 255, 255};

    // determine if we're interpolating to positive or negative
    struct nk_color const target = ( value > 0.0f ) ? positive : negative;
    float const t = (value > 0.0f) ? value : -value;

    // interpolate through target and return new color
    return (struct nk_color) {
        .r = (nk_byte) (neutral.r + t * (target.r - neutral.r)),
        .g = (nk_byte) (neutral.g + t * (target.g - neutral.g)),
        .b = (nk_byte) (neutral.b + t * (target.b - neutral.b)),
        .a = 255
    };
}

/**
 * attraction_factor_button
 *
 * @brief Builds a button style whose fill color encodes an attraction weight.
 *
 * Copies the context's base button style and sets all three visual states to the
 * color returned by attraction_color(*value), so a matrix cell visually reflects
 * its weight (red = repel, gray = neutral, blue = attract).
 *
 * @param context  The Nuklear context whose base button style is copied.
 * @param value    Pointer to the attraction weight to visualize (read, not modified).
 * @return         A configured nk_style_button to pass to nk_button_*_styled().
 *
 * @see  attraction_color()
 */
struct nk_style_button attraction_factor_button(struct nk_context *context, float *value) {
    struct nk_style_button button = context->style.button;

    // background
    struct nk_color target_color = attraction_color(*value);
    button.normal = (struct nk_style_item) { .type = NK_STYLE_ITEM_COLOR, .data.color = target_color };
    button.hover  = (struct nk_style_item) { .type = NK_STYLE_ITEM_COLOR, .data.color = target_color };
    button.active = (struct nk_style_item) { .type = NK_STYLE_ITEM_COLOR, .data.color = target_color };

    return button;
}