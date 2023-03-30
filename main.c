#include "nanogui.h"


int main() {
    int width = 640, height = 480;
    nano_gui_create_fixed_size_window(width, height);

    // Main loop
    while (nano_gui_process_events()) {
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                nano_gui_draw_pixel(x, y, 255, 0, 255);
            }
        }
    }
    return 0;
}