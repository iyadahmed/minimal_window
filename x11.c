// Based on https://en.wikibooks.org/wiki/X_Window_Programming/Xlib and https://stackoverflow.com/a/54528360/8094047

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h> // thanks! https://stackoverflow.com/a/38555194/8094047

static Display *display;
static Window window;
static int screen;
static int global_width, global_height;
static XImage *image;

static unsigned long rgb_to_ulong(uint8_t r, uint8_t g, uint8_t b) { return 65536 * b + 256 * g + r; }

void nano_gui_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  unsigned long color = rgb_to_ulong(r, g, b);
  XPutPixel(image, x, y, color);
}

void nano_gui_create_fixed_size_window(int width, int height) {
  global_width = width;
  global_height = height;

  /* open connection with the server */
  display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }

  screen = DefaultScreen(display);

  /* create window */
  window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, width, height, 1,
                               BlackPixel(display, screen), WhitePixel(display, screen));

  /* process window close event through event handler so XNextEvent does not fail */
  Atom del_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
  XSetWMProtocols(display, window, &del_window, 1);

  /* select kind of events we are interested in */
  XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask);

  /* display the window */
  XMapWindow(display, window);

  // Wait for MapNotify
  for (;;) {
    XEvent e;
    XNextEvent(display, &e);
    if (e.type == MapNotify)
      break;
  }

  image = XGetImage(display, window, 0, 0, width, height, AllPlanes, ZPixmap);
}

bool nano_gui_process_events() {
  XEvent event;
  XNextEvent(display, &event);
  switch (event.type) {
  case KeyPress:
    /* FALLTHROUGH */

  case ClientMessage:
    /* destroy window */
    XDestroyWindow(display, window);
    /* close connection to server */
    XCloseDisplay(display);
    return false;

  case Expose:
    /* draw the window */
    //            XFillRectangle(display, window, DefaultGC(display, screen), 0, 0, global_width, global_height);
    XPutImage(display, window, DefaultGC(display, screen), image, 0, 0, 0, 0, global_width, global_height);

    /* NO DEFAULT */
  }

  return true;
}
