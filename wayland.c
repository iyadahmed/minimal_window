#include <stdio.h>
#include <wayland-client.h>

void nano_gui_create_fixed_size_window(int width, int height) {
    struct wl_display *display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display.\n");
        return;
    }
    fprintf(stderr, "Connection established!\n");

    // TODO: disconnect when closing window
    wl_display_disconnect(display);
}