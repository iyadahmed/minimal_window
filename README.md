Build Status:  
[![](http://github-actions.40ants.com/iyadahmed/minimal_window/matrix.svg)](https://github.com/iyadahmed/minimal_window)

# Minimal Window
A very small C library for putting pixels on the screen, perfect for software raytracing projects.

## How to include in your CMake project
1. Install the relevant libraries and runtimes: see [Supported Platforms](#supported-platforms)
2. Clone or download the repo. to your project
3. Use `add_subdirectory("/path/to/minimal_window")` with correct path
4. Link to minimal_window: `target_link_libraries(your_target PRIVATE minimal_window)`

Alternatively, you can use `ExternalProject`, `FetchContent`, [CPM](https://github.com/cpm-cmake/CPM.cmake), or any other method that you are comfortable with, minimal_window is a modern CMake project and should be very easy to include

## Supported Platforms
- [x] Win32 (Windows)
- [x] Wayland (Linux):  
      Wayland development packages are needed for building (for example on Ubuntu: `sudo apt install libwayland-dev wayland-protocols`), you also need a running Wayland session at runtime.
      You also need pkg-config `sudo apt install pkg-config`.
- [x] X11 (Linux and macOS):  
      * **Linux**: X11 development packages are needed for building (for example on Ubuntu: `sudo apt install libx11-dev`), you also need a running X11 session at runtime.  
      * **macOS**: [XQuartz](https://www.xquartz.org/) is needed at runtime.
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
