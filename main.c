#include "nanogui.h"


int main() {
    nano_gui_create_fixed_size_window(800, 600);

    // Main loop
    while (nano_gui_process_events()) {
    }
    return 0;
}