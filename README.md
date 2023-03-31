# NanoGUI
A very small GUI library for putting pixels on the screen, perfect for software raytracing projects.

## Supported Platforms
- [x] Win32 (Windows)
- [ ] X11 (Linux, and macOS via XQuartz)

## Supported Compilers
- [x] GCC on Windows (MSYS2 Mingw64)
- [ ] MSVC


## API
```c
void nano_gui_create_fixed_size_window(int width, int height);

bool nano_gui_process_events();

void nano_gui_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
```

## Example: filling window with white

```c
#include "nanogui.h"
int main() {
  int width = 800, height = 600;
  nano_gui_create_fixed_size_window(width, height);
  while (nano_gui_process_events()) {
    for (int i = 0; i < width; i++)
      for (int j = 0; j < height; j++)
        nano_gui_draw_pixel(i, j, 255, 255, 255);
  }
  return 0;
}
```

## Example: raytracing
See [WonderBox](https://github.com/iyadahmed/WonderBox/blob/main/main.c)
