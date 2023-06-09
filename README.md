Build Status:  
[![](http://github-actions.40ants.com/iyadahmed/minimal_window/matrix.svg)](https://github.com/iyadahmed/minimal_window)

# Minimal Window
A very small C library for putting pixels on the screen, perfect for software raytracing projects.

## Supported Platforms
- [x] Win32 (Windows)
- [x] Wayland (Linux) (for example on Ubuntu: `sudo apt install libwayland-dev wayland-protocols` is needed for building)
- [x] X11 (Linux, (for example on Ubuntu: `sudo apt install libx11-dev` is needed for building, on macOS [XQuartz](https://www.xquartz.org/) is needed at runtime)
- [ ] Native macOS

## Tested Compilers
- [x] GCC
- [x] MSVC
- [x] Apple Clang
- [x] Clang


## API
```c
void minimal_window_create_fixed_size_window(int width, int height);

bool minimal_window_process_events();

void minimal_window_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
```

## Example: filling window with white

```c
#include <minimal_window.h>
int main() {
  int width = 800, height = 600;
  minimal_window_create_fixed_size_window(width, height);
  while (minimal_window_process_events()) {
    for (int i = 0; i < width; i++)
      for (int j = 0; j < height; j++)
        minimal_window_draw_pixel(i, j, 255, 255, 255);
  }
  return 0;
}
```

## Example: raytracing
https://github.com/iyadahmed/minimal_window_raytracing/blob/main/main.c
