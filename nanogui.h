#ifndef NANOGUI_NANOGUI_H
#define NANOGUI_NANOGUI_H

#include <stdbool.h>
#include <stdint.h>

void nano_gui_create_fixed_size_window(int width, int height);

bool nano_gui_process_events();

void nano_gui_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

#endif // NANOGUI_NANOGUI_H
