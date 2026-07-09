/**
 * @file main.c
 * @brief Program entry point.
 *
 * Wires together the application lifecycle: initialize the window and GL
 * context, the simulation, and the GUI; run the main loop; then clean up.
 */

#include "application.h"

/**
 * main
 *
 * @brief Program entry point that sets up, runs, and tears down the application.
 *
 * Initializes the application, simulation, and GUI in order, runs the main loop
 * until exit, then releases all resources. Bails out early with a non-zero exit
 * code if any initialization step fails.
 *
 * @return  0 on a clean exit, 1 if initialization failed.
 */
int main(void) {
    // Initialization
    application_t application = { 0 };
    if (!init_application(&application)) return 1;
    if (!init_simulation(&application))  return 1;
    if (!init_gui(&application))         return 1;

    mainloop(&application);

    // Clean up
    destroy_application(&application);

    return 0;
}
