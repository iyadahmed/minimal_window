#ifndef MINIMAL_WINDOW_H
#define MINIMAL_WINDOW_H

#include <stdbool.h>
#include <stdint.h>

void minimal_window_create_fixed_size_window(int width, int height);

bool minimal_window_process_events();

void minimal_window_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

void minimal_window_request_update();

#endif // MINIMAL_WINDOW_H
