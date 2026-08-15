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
#include "nuklear.h"

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

/**
 * hsv_to_rgb
 *
 * @brief Converts an HSV color to Nuklear's 8-bit RGBA color type.
 *
 * @param hue         Hue in degrees [0, 360).
 * @param saturation  Saturation in [0, 1].
 * @param value       Value/brightness in [0, 1].
 * @return            The equivalent color with alpha fixed at 255.
 *
 * @note This is a static internal helper for rainbow_button().
 */
static struct nk_color hsv_to_rgb(float hue, float saturation, float value) {
    float red        = 0.0f;
    float green      = 0.0f;
    float blue       = 0.0f;

    float chroma = value * saturation;
    float interpol = chroma * (1.0f - fabsf(fmodf(hue / 60.0f, 2.0f) - 1.0f));
    float brightness = value - chroma;

    if (hue < 60.0f) {
        red = chroma; green = interpol; blue = 0;
    }
    else if (hue < 120.0f) {
        red = interpol; green = chroma; blue = 0;
    }
    else if (hue < 180.0f) {
        red = 0; green = chroma; blue = interpol;
    }
    else if (hue < 240.0f) {
        red = 0; green = interpol; blue = chroma;
    }
    else if (hue < 300.0f) {
        red = interpol; green = 0; blue = chroma;
    }
    else {
        red = chroma; green = 0; blue = interpol;
    }

    hue += 0.2f;
    if (hue >= 360.0f) hue = 0.0f;

    return (struct nk_color) {
        ( (red + brightness)   * 255.0f),
        ( (green + brightness) * 255.0f),
        ( (blue + brightness)  * 255.0f),
        255
    };
}

/**
 * rainbow_button
 *
 * @brief Builds a circular button style whose color cycles through the hue wheel.
 *
 * Advances a function-local hue by a fixed step on every call and derives a color
 * from it via hsv_to_rgb(). When fill is true, the swatch is solid-filled with
 * that color (used for the "apply to all classes" color swatch); when false, it
 * stays black with a colored border instead (used for the attraction matrix's
 * "select all cells" corner button).
 *
 * @param context  The Nuklear context whose base button style is copied.
 * @param fill     True to solid-fill the button with the color; false for a
 *                 black button with a colored border.
 * @return         A configured nk_style_button to pass to nk_button_*_styled().
 *
 * @note The hue advances on every call regardless of caller, since it is stored
 *       in a single function-local static shared by all call sites.
 */
struct nk_style_button rainbow_button(struct nk_context *context, bool fill) {
    static float hue = 0.0f;
    struct nk_color color = hsv_to_rgb(hue, 1.0f, 1.0f);
    struct nk_style_button button = circular_button(context, (fill) ? color : (struct nk_color) { 0, 0, 0, 255 });

    if (!fill) {
        button.border_color = color;
        button.border = 1.0f;
    }

    hue += 0.25f;
    if (hue >= 360.0f) hue = 0.0f;

    return button;
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
 * Copies the context's base button style and colors normal/active from
 * attraction_color(*value), so a matrix cell visually reflects its weight
 * (red = repel, gray = neutral, blue = attract); hover uses the same color at
 * reduced alpha so hovered cells stay visually distinct.
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
    struct nk_color hovered = { target_color.r, target_color.g, target_color.b, target_color.a - 55 };
    button.normal = (struct nk_style_item) { .type = NK_STYLE_ITEM_COLOR, .data.color = target_color };
    button.hover  = (struct nk_style_item) { .type = NK_STYLE_ITEM_COLOR, .data.color = hovered };
    button.active = (struct nk_style_item) { .type = NK_STYLE_ITEM_COLOR, .data.color = target_color };

    return button;
}

#define SLIDER_WIDGET_HEIGHT    20

/**
 * uint_variable_slider
 *
 * @brief Builds a labeled row combining an integer slider with a typeable textbox.
 *
 * Lays out a label, an nk_slider_int, and a text field side by side; both control
 * the same underlying value. Dragging the slider updates *buffer immediately;
 * typing a number into the textbox and pressing Enter parses and clamps it to
 * [min, max] before committing. The textbox's displayed text is resynced from
 * *buffer whenever the field is not actively being edited, so it never lags
 * behind a slider drag or an external change to *buffer.
 *
 * @param context      The Nuklear context to build the widgets in.
 * @param label        Row label shown to the left of the slider.
 * @param min          Minimum allowed value (inclusive).
 * @param buffer       Pointer to the value being edited.
 * @param max          Maximum allowed value (inclusive).
 * @param text_buffer  Caller-owned, persistent char[TEXT_MAX_SIZE] backing the
 *                     textbox; must be unique per call site (e.g. a `static`
 *                     local), since sharing one buffer across call sites would
 *                     make them overwrite each other's displayed text.
 * @return             True if the value changed this frame (via drag or commit).
 */
bool uint_variable_slider(struct nk_context *context, char const * const label, uint32_t min, uint32_t *buffer, uint32_t max, char *text_buffer) {
    bool result = false;

    nk_layout_row(context, NK_DYNAMIC, SLIDER_WIDGET_HEIGHT, 3, (float const []) { 0.2f, 0.6f, 0.2f });

    nk_label(context, label, NK_TEXT_ALIGN_LEFT);
    if (nk_slider_int(context, min, (int *) buffer, max, 1))
        result = true;
    nk_flags textbox_state = nk_edit_string_zero_terminated(context, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER | NK_EDIT_AUTO_SELECT, text_buffer, TEXT_MAX_SIZE, nk_filter_decimal);
    if (textbox_state & NK_EDIT_COMMITTED) {
        double committed_value = nk_strtod(text_buffer, NULL);
        if      (committed_value < min) committed_value = min;
        else if (committed_value > max) committed_value = max;

        *buffer = (int) committed_value;
        result = true;
    }
    else if (!(textbox_state & NK_EDIT_ACTIVE)) snprintf(text_buffer, TEXT_MAX_SIZE, "%u", *buffer);

    return result;
}

/**
 * float_variable_slider
 *
 * @brief Builds a labeled row combining a float slider with a typeable textbox.
 *
 * Same combined slider+textbox pattern as uint_variable_slider(), but for a
 * floating-point value: dragging the slider steps by step, and typing a number
 * into the textbox and pressing Enter parses and clamps it to [min, max] before
 * committing.
 *
 * @param context      The Nuklear context to build the widgets in.
 * @param label        Row label shown to the left of the slider.
 * @param min          Minimum allowed value (inclusive).
 * @param buffer       Pointer to the value being edited.
 * @param max          Maximum allowed value (inclusive).
 * @param step         Slider drag increment.
 * @param text_buffer  Caller-owned, persistent char[TEXT_MAX_SIZE] backing the
 *                     textbox; must be unique per call site.
 * @return             True if the value changed this frame (via drag or commit).
 *
 * @see uint_variable_slider()
 */
bool float_variable_slider(struct nk_context *context, char const *label, float min, float *buffer, float max, float step, char *text_buffer) {
    bool result = false;

    nk_layout_row(context, NK_DYNAMIC, SLIDER_WIDGET_HEIGHT, 3, (float const []) { 0.25f, 0.55f, 0.2f });

    nk_label(context, label, NK_TEXT_ALIGN_LEFT);
    if (nk_slider_float(context, min, buffer, max, step))
        result = true;
    nk_flags textbox_state = nk_edit_string_zero_terminated(context, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER | NK_EDIT_AUTO_SELECT, text_buffer, TEXT_MAX_SIZE, nk_filter_float);
    if (textbox_state & NK_EDIT_COMMITTED) {
        double committed_value = nk_strtod(text_buffer, NULL);
        if      (committed_value < min) committed_value = min;
        else if (committed_value > max) committed_value = max;

        *buffer = (float) committed_value;
        result = true;
    }
    else if (!(textbox_state & NK_EDIT_ACTIVE)) snprintf(text_buffer, TEXT_MAX_SIZE, "%g", *buffer);

    return result;
}

#define COLOR_PICKER_AREA_HEIGHT    225

/**
 * extended_color_picker
 *
 * @brief Builds an RGBA color-picker panel: an HSV wheel plus per-channel int fields.
 *
 * Draws Nuklear's built-in HSV color wheel bound directly to color, then four
 * nk_property_int fields (red/green/blue/alpha, 0-255) that mirror and can also
 * edit the same color. Editing a channel field writes back into the matching
 * float channel of color, scaled by COLOR_RANGE.
 *
 * @param context  The Nuklear context to build the widgets in.
 * @param color    The RGBA color being edited in place, channels in [0, 1].
 */
void extended_color_picker(struct nk_context *context, float color[NUM_CHANNELS]) {
    nk_layout_row_dynamic(context, 220, 1);
    nk_color_pick(context, (struct nk_colorf *) color, NK_RGBA);

    int red   = (int) (COLOR_RANGE * color[0]);
    int green = (int) (COLOR_RANGE * color[1]);
    int blue  = (int) (COLOR_RANGE * color[2]);
    int alpha = (int) (COLOR_RANGE * color[3]);

    nk_layout_row_dynamic(context, 25, 1);
    if (nk_property_int(context, "red",   0, &red,   255, 1, 5))
        color[0] = (float) red / COLOR_RANGE;
    if (nk_property_int(context, "green", 0, &green, 255, 1, 5))
        color[1] = (float) green / COLOR_RANGE;
    if (nk_property_int(context, "blue",  0, &blue,  255, 1, 5))
        color[2] = (float) blue / COLOR_RANGE;
    if (nk_property_int(context, "alpha", 0, &alpha, 255, 1, 5))
        color[3] = (float) alpha / COLOR_RANGE;
}

#define MATRIX_CELL_SIZE      28.0f
#define MATRIX_CELL_PADDING    3.0f


/**
 * create_grid
 *
 * @brief Builds the interactive (nclass+1) x (nclass+1) attraction matrix grid.
 *
 * Lays out a square grid of cells in a manually positioned space:
 *  - The top-left corner is a cycling rainbow button; clicking it selects the
 *    whole matrix (sets output->universal and fills the selection mask).
 *  - The rest of the top row and left column are circular class-color headers,
 *    drawn from palette, one per class.
 *  - Every other cell (r > 0, c > 0) is an attraction-factor button, colored by
 *    attraction_factor_button() from its weight in values[]. Clicking a cell
 *    selects it: a plain click replaces the selection with just that cell,
 *    while Ctrl-click adds it to the existing selection so multiple cells can
 *    be edited together. Selected cells (and all cells when universal is set)
 *    are drawn with a white border.
 *
 * @param context     The Nuklear context to build the grid in.
 * @param values      Flat, MAX_NUM_CLASSES-stride, row-major attraction weight
 *                    array; read to color cells.
 * @param palette     Per-class RGBA colors, used for the row/column headers.
 * @param dimensions  Grid dimension, i.e. nclass + 1 (for the header row/column).
 * @param output      Selection state updated on click: universal, rowidx/colidx
 *                    of the most recently clicked cell, and the selection[] mask.
 *
 * @note output->selection[] is indexed the same way as values[]
 *       (matrix_idx = (row-1) * MAX_NUM_CLASSES + (col-1)); it is cleared only
 *       on a plain (non-Ctrl) click, so Ctrl-clicks accumulate across frames.
 */
void create_grid(struct nk_context *context, float *values, float (*palette)[NUM_CHANNELS], uint8_t const dimensions, active_cell_t *output) {
    float const step = MATRIX_CELL_SIZE + MATRIX_CELL_PADDING;
    float const span = dimensions * MATRIX_CELL_SIZE + (dimensions - 1) * MATRIX_CELL_PADDING;
    float const available = nk_window_get_content_region_size(context).x;
    float       x0 = (available - span) * 0.5f;
    
    nk_layout_space_begin(context, NK_STATIC, span, dimensions * dimensions);

    for (uint8_t r = 0; r < dimensions; r++) {
        for (uint8_t c = 0; c < dimensions; c++) {
            nk_layout_space_push(context, nk_rect(x0 + c * step, r * step, MATRIX_CELL_SIZE, MATRIX_CELL_SIZE));

            if (r == 0 && c == 0) { // Representation for all particles
                struct nk_style_button all_colors = rainbow_button(context, false);
                if (nk_button_label_styled(context, &all_colors, EMPTY_STRING)) {
                    output->universal = true;
                    memset(output->selection, true, MAX_NUM_CLASSES * MAX_NUM_CLASSES * sizeof(uint8_t));
                }
            }
            else if (r > 0 && c > 0) { // attraction factor cell
                uint8_t matrix_idx = (r - 1) * MAX_NUM_CLASSES + (c - 1);
                struct nk_style_button factor = attraction_factor_button(context, &values[matrix_idx]);
                if (output->universal || output->selection[matrix_idx] == true)
                    factor.border_color = (struct nk_color) { 255, 255, 255, 255 };
                if (nk_button_label_styled(context, &factor, EMPTY_STRING)) {
                    output->rowidx = r;
                    output->colidx = c;
                    output->universal = false;
                    if (!(SDL_GetModState() & SDL_KMOD_CTRL)) { // Check 
                        memset(output->selection, false, MAX_NUM_CLASSES * MAX_NUM_CLASSES * sizeof(bool));
                        output->selection[matrix_idx] = true;
                    }
                    else {
                        output->selection[matrix_idx] = !(output->selection[matrix_idx]);
                    }
                }
            }
            else { // header cell
                uint8_t palette_idx = (r == 0) ? (c - 1) : (r - 1); // determine the header for row and col
                struct nk_style_button header = circular_button(context, (struct nk_color) {
                    .r = (nk_byte) (COLOR_RANGE * palette[palette_idx][0]),
                    .g = (nk_byte) (COLOR_RANGE * palette[palette_idx][1]),
                    .b = (nk_byte) (COLOR_RANGE * palette[palette_idx][2]),
                    .a = (nk_byte) (COLOR_RANGE) } );
                    nk_button_label_styled(context, &header, EMPTY_STRING);
            }
        }
    }
    
    nk_layout_space_end(context);
}

#define WEIGHT_SCALAR          100

/**
 * apply_attraction_value
 *
 * @brief Writes a single scaled attraction value into every cell a selection covers.
 *
 * If cell_data->universal is set, writes to the whole matrix (attraction->length
 * entries); otherwise writes to every index flagged true in cell_data->selection,
 * which may be one cell or many. scaled_value is in the display range [-100, 100]
 * used by cell_editor()'s slider/textbox and is divided by WEIGHT_SCALAR to land
 * in the matrix's actual [-1, 1] weight range.
 *
 * @param attraction    The attraction matrix to write into.
 * @param cell_data     Selection state describing which cell(s) to update.
 * @param scaled_value  New value in [-100, 100] (i.e. value * WEIGHT_SCALAR).
 *
 * @note This is a static internal helper for cell_editor().
 */
static void apply_attraction_value(attraction_t *attraction, active_cell_t const *cell_data, int scaled_value) {
    float const value = (float) scaled_value / WEIGHT_SCALAR;
    if (cell_data->universal) {
        for (size_t i = 0; i < attraction->length; ++i)
            attraction->matrix[i] = value;
    }
    else {
        for (size_t i = 0; i < MAX_NUM_CLASSES * MAX_NUM_CLASSES; ++i)
            if (cell_data->selection[i]) attraction->matrix[i] = value;
    }
}

/**
 * cell_editor
 *
 * @brief Builds the value editor below the grid for the current selection.
 *
 * Draws two swatches (active/affected) linked by an arrow to show what's being
 * edited, then a combined slider+textbox for the attraction weight, scaled to
 * the display range [-100, 100]. When exactly one cell is selected, the field
 * is resynced from that cell's stored value each frame; with zero, multiple, or
 * a universal selection there's no single authoritative value to show, so the
 * field is left at whatever was last set. Any change (slider drag or textbox
 * commit) is applied to every cell the current selection covers via
 * apply_attraction_value().
 *
 * @param context     The Nuklear context to build the widgets in.
 * @param attraction  The attraction matrix being edited.
 * @param cell_data   Selection state from create_grid(), passed by value (a
 *                    snapshot for this frame).
 * @param active      Style for the "active" (row-class) swatch button.
 * @param affected    Style for the "affected" (column-class) swatch button.
 * @return            True if the attraction matrix changed this frame.
 *
 * @see apply_attraction_value(), create_grid()
 */
bool cell_editor(struct nk_context *context, attraction_t *attraction, active_cell_t cell_data, struct nk_style_button active, struct nk_style_button affected) {
    bool is_dirty = false;

    nk_layout_row(context, NK_DYNAMIC, MATRIX_CELL_SIZE, 5, (float const []) {0.1f, 0.1f, 0.1f, 0.5f, 0.2f});
    nk_button_label_styled(context, &active, EMPTY_STRING);

    struct nk_rect bounds;
    nk_widget(&bounds, context);
    struct nk_vec2 arrow[3];
    nk_triangle_from_direction(arrow, bounds, 8.0f, 8.0f, NK_RIGHT);
    nk_fill_triangle(nk_window_get_canvas(context),
                      arrow[0].x, arrow[0].y,
                      arrow[1].x, arrow[1].y,
                      arrow[2].x, arrow[2].y,
                      context->style.text.color);
    nk_button_label_styled(context, &affected, EMPTY_STRING);

    static int new_attraction_value = 0;
    if (!cell_data.universal) {
        int selected_idx = -1;
        int selected_count = 0;
        for (size_t i = 0; i < MAX_NUM_CLASSES * MAX_NUM_CLASSES; ++i) {
            if (cell_data.selection[i]) {
                selected_idx = (int) i;
                ++selected_count;
            }
        }
        if (selected_count == 1)
            new_attraction_value = (int) (attraction->matrix[selected_idx] * WEIGHT_SCALAR);
    }

    if (nk_slider_int(context, -100, &new_attraction_value, 100, 1)) {
        apply_attraction_value(attraction, &cell_data, new_attraction_value);
        is_dirty = true;
    }

    static char factor[TEXT_MAX_SIZE] = { '\0' };
    nk_flags textbox_state = nk_edit_string_zero_terminated(context, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER | NK_EDIT_AUTO_SELECT, factor, TEXT_MAX_SIZE, nk_filter_decimal);
    if (textbox_state & NK_EDIT_COMMITTED) {
        double committed_value = nk_strtod(factor, NULL);
        if      (committed_value < -100) committed_value = -100;
        else if (committed_value >  100) committed_value = 100;

        new_attraction_value = (int) committed_value;
        apply_attraction_value(attraction, &cell_data, new_attraction_value);
        is_dirty = true;
    }
    else if (!(textbox_state & NK_EDIT_ACTIVE)) snprintf(factor, TEXT_MAX_SIZE, "%d", new_attraction_value);

    return is_dirty;
}

/**
 * grid_preset_randomize
 *
 * @brief Builds the attraction-matrix preset combobox and randomize button.
 *
 * The combobox overwrites the whole matrix with a built-in named preset from
 * matrix_presets[]; the randomize button refills every entry with a fresh
 * random weight in [-1, 1]. Either action replaces the entire matrix,
 * independent of any current cell selection.
 *
 * @param context     The Nuklear context to build the widgets in.
 * @param attraction  The attraction matrix to overwrite.
 * @return            True if the matrix changed this frame.
 */
bool grid_preset_randomize(struct nk_context *context, attraction_t *attraction) {
    bool is_dirty = false;
    // attraction presets
    // combobox and randomize button
    nk_layout_row(context, NK_DYNAMIC, 25, 2, (float const []) { 0.7f, 0.3f });
    if (nk_combo_begin_label(context, "Attraction Presets",
                              nk_vec2(DEFAULT_PANEL_WIDTH - PANEL_CONTENT_RIGHT_PADDING, 200))) {
        nk_layout_row_dynamic(context, 25, 1);
        for (uint8_t i = 0; i < MATRIX_PRESET_COUNT; i++) {
            if (nk_combo_item_label(context, matrix_preset_names[i], NK_TEXT_LEFT)) {
                memcpy(attraction->matrix, matrix_presets[i], sizeof(float) * attraction->length);
                is_dirty = true;
            }
        }
        nk_combo_end(context);
    }
    if (nk_button_label(context, "Randomize")) {
        for (size_t i = 0; i < attraction->length; i++)
            attraction->matrix[i] = SDL_randf() * 2.0f - 1.0f;
        is_dirty = true;
    }

    return is_dirty;
}
