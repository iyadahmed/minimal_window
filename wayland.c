// Based on https://wayland-book.com/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <wayland-client.h>

#include "shm.h"


uint32_t *pixels;
static struct wl_display *display;
struct wl_registry *registry;
int global_width, global_height;
struct wl_surface *surface;

struct state_t {
    struct wl_shm *shm;
    struct wl_compositor *compositor;
};

static uint32_t rgb_to_u32(uint8_t r, uint8_t g, uint8_t b) {
    // Credit: https://stackoverflow.com/a/39979191/8094047
    uint8_t alpha = 255;
    return (alpha << 24) + (r << 16) + (g << 8) + b;
}

void nano_gui_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    pixels[y * global_width + x] = rgb_to_u32(r, g, b);
}

static void
registry_handle_global(void *data, struct wl_registry *wl_registry,
                       uint32_t name, const char *interface, uint32_t version) {
    struct state_t *state = data;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        state->compositor = wl_registry_bind(
                wl_registry, name, &wl_compositor_interface, 4);
        state->shm = wl_registry_bind(
                wl_registry, name, &wl_shm_interface, 1);
    }

    printf("interface: '%s', version: %d, name: %d\n",
           interface, version, name);
}

static void
registry_handle_global_remove(void *data, struct wl_registry *wl_registry,
                              uint32_t name) {
    // This space deliberately left blank
}

static const struct wl_registry_listener
        registry_listener = {
        .global = registry_handle_global,
        .global_remove = registry_handle_global_remove,
};

void nano_gui_create_fixed_size_window(int width, int height) {
    global_width = width;
    global_height = height;

    display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display.\n");
        return;
    }
    fprintf(stderr, "Connection established!\n");

    registry = wl_display_get_registry(display);
    struct state_t state = {0};
    wl_registry_add_listener(registry, &registry_listener, &state);
    surface = wl_compositor_create_surface(state.compositor);

    // Create shared memory pool
    const int stride = width * 4;
    const int shm_pool_size = height * stride * 2;
    int fd = allocate_shm_file(shm_pool_size);
    uint8_t *pool_data = mmap(NULL, shm_pool_size,
                              PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct wl_shm_pool *pool = wl_shm_create_pool(state.shm, fd, shm_pool_size);

    int index = 0;
    int offset = height * stride * index;
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, offset,
                                                         width, height, stride, WL_SHM_FORMAT_XRGB8888);
    pixels = (uint32_t *) &pool_data[offset];
    wl_surface_attach(surface, buffer, 0, 0);
}

bool nano_gui_process_events() {
    if (wl_display_dispatch(display) != -1) {
        wl_surface_damage(surface, 0, 0, UINT32_MAX, UINT32_MAX);
        wl_surface_commit(surface);
        return true;
    } else {
        wl_display_disconnect(display);
        return false;
    }
}