// Based on code from the Wayland book:
// https://wayland-book.com/xdg-shell-basics/example-code.html ability to move
// and close the window from "hello-wayland":
// https://github.com/emersion/hello-wayland

#define _POSIX_C_SOURCE 200112L

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/input-event-codes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>

#include "xdg-shell-client-protocol.h"

struct client_state {
  /* Globals */
  struct wl_display *wl_display;
  struct wl_registry *wl_registry;
  struct wl_shm *wl_shm;
  struct wl_compositor *wl_compositor;
  struct xdg_wm_base *xdg_wm_base;
  /* Objects */
  struct wl_surface *wl_surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
};

/* Global variables */
static struct client_state global_client_state = {0};
static uint32_t *global_image_data;
static int global_data_size;
static int global_width, global_height;
static struct wl_buffer *global_wl_buffer;
static bool global_running = false;

static uint32_t rgb_to_u32(uint8_t r, uint8_t g, uint8_t b) {
  /* Credit: https://stackoverflow.com/a/39979191/8094047 */
  uint8_t alpha = 255;
  return (alpha << 24) + (r << 16) + (g << 8) + b;
}

void nano_gui_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t *pixel = (uint32_t *)global_image_data;
  pixel += y * global_width + x;
  *pixel = rgb_to_u32(r, g, b);
}

/* Shared memory support code */
static void randname(char *buf) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  long r = ts.tv_nsec;
  for (int i = 0; i < 6; ++i) {
    buf[i] = 'A' + (r & 15) + (r & 16) * 2;
    r >>= 5;
  }
}

static int create_shm_file(void) {
  int retries = 100;
  do {
    char name[] = "/wl_shm-XXXXXX";
    randname(name + sizeof(name) - 7);
    --retries;
    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd >= 0) {
      shm_unlink(name);
      return fd;
    }
  } while (retries > 0 && errno == EEXIST);
  return -1;
}

static int allocate_shm_file(size_t size) {
  int fd = create_shm_file();
  if (fd < 0)
    return -1;
  int ret;
  do {
    ret = ftruncate(fd, size);
  } while (ret < 0 && errno == EINTR);
  if (ret < 0) {
    close(fd);
    return -1;
  }
  return fd;
}

static void do_nothing() {}

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel) { global_running = false; }

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = do_nothing,
    .close = xdg_toplevel_handle_close,
};

static void pointer_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time,
                                  uint32_t button, uint32_t state) {
  struct wl_seat *seat = data;

  if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED) {
    xdg_toplevel_move(global_client_state.xdg_toplevel, seat, serial);
  }
}

static const struct wl_pointer_listener pointer_listener = {
    .enter = do_nothing,
    .leave = do_nothing,
    .motion = do_nothing,
    .button = pointer_handle_button,
    .axis = do_nothing,
};

static void seat_handle_capabilities(void *data, struct wl_seat *seat, uint32_t capabilities) {
  if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
    struct wl_pointer *pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(pointer, &pointer_listener, seat);
  }
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_handle_capabilities,
};

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer) {
  /* Sent by the compositor when it's no longer using this buffer */
  wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};

static void xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
  xdg_surface_ack_configure(xdg_surface, serial);
  wl_surface_commit(global_client_state.wl_surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_handle_configure,
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static void registry_global(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface,
                            uint32_t version) {
  struct client_state *state = data;
  if (strcmp(interface, wl_shm_interface.name) == 0) {
    state->wl_shm = wl_registry_bind(wl_registry, name, &wl_shm_interface, 1);

  } else if (strcmp(interface, wl_compositor_interface.name) == 0) {
    state->wl_compositor = wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);

  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    state->xdg_wm_base = wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(state->xdg_wm_base, &xdg_wm_base_listener, state);

  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    struct wl_seat *seat = wl_registry_bind(global_client_state.wl_registry, name, &wl_seat_interface, 1);
    wl_seat_add_listener(seat, &seat_listener, NULL);
  }
}

static const struct wl_registry_listener wl_registry_listener = {
    .global = registry_global,
    .global_remove = do_nothing,
};

void nano_gui_create_fixed_size_window(int width, int height) {
  global_width = width;
  global_height = height;
  global_client_state.wl_display = wl_display_connect(NULL);

  if (global_client_state.wl_display == NULL) {
    fprintf(stderr, "failed to create display\n");
    return;
  }

  global_client_state.wl_registry = wl_display_get_registry(global_client_state.wl_display);
  wl_registry_add_listener(global_client_state.wl_registry, &wl_registry_listener, &global_client_state);
  wl_display_roundtrip(global_client_state.wl_display);

  if (global_client_state.wl_shm == NULL || global_client_state.wl_compositor == NULL ||
      global_client_state.xdg_wm_base == NULL) {
    /* These values should have been set after a wl_display_roundtrip, if not print error and return */
    fprintf(stderr, "no wl_shm, wl_compositor or xdg_wm_base support\n");
    return;
  }

  global_client_state.wl_surface = wl_compositor_create_surface(global_client_state.wl_compositor);
  global_client_state.xdg_surface =
      xdg_wm_base_get_xdg_surface(global_client_state.xdg_wm_base, global_client_state.wl_surface);
  global_client_state.xdg_toplevel = xdg_surface_get_toplevel(global_client_state.xdg_surface);

  xdg_toplevel_set_title(global_client_state.xdg_toplevel, "Example client");

  xdg_surface_add_listener(global_client_state.xdg_surface, &xdg_surface_listener, &global_client_state);
  xdg_toplevel_add_listener(global_client_state.xdg_toplevel, &xdg_toplevel_listener, NULL);
  wl_buffer_add_listener(global_wl_buffer, &wl_buffer_listener, NULL);

  wl_surface_commit(global_client_state.wl_surface);
  wl_display_roundtrip(global_client_state.wl_display);

  /* Create buffer */
  {
    int stride = width * 4;
    global_data_size = stride * height;

    int fd = allocate_shm_file(global_data_size);
    if (fd == -1) {
      fprintf(stderr, "failed to allocate shared memory file\n");
      return;
    }

    global_image_data = mmap(NULL, global_data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (global_image_data == MAP_FAILED) {
      close(fd);
      return;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(global_client_state.wl_shm, fd, global_data_size);
    global_wl_buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);
  }

  wl_surface_attach(global_client_state.wl_surface, global_wl_buffer, 0, 0);
  wl_surface_commit(global_client_state.wl_surface);

  global_running = true;
}

bool nano_gui_process_events() {
  if (wl_display_dispatch(global_client_state.wl_display) && global_running) {
    return true;
  } else {
    /* Cleanup */
    munmap(global_image_data, global_data_size);
    xdg_toplevel_destroy(global_client_state.xdg_toplevel);
    xdg_surface_destroy(global_client_state.xdg_surface);
    wl_surface_destroy(global_client_state.wl_surface);
    return false;
  }
}
