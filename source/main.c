#include <stdio.h>

#include "SDL.h"

#include "../include/application.h"

int main(void) {
    // Initialization
    application_t application = { 0 };
    if (!init_application(&application)) return 1;
    if (!init_simulation(&application))  return 1;

    mainloop(&application);

    // Clean up
    destroy_application(&application);

    return 0;
}
