// Based on: https://samulinatri.com/things/win32-drawing-pixels/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#define UNICODE

#include <windows.h>

// NOTE: WS_CAPTION must be used otherwise AdjustWindowSize will give wrong results
#define FIXED_SIZE_WINDOW_STYLE (WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION)

static BITMAPINFO bitmap_info;
static int global_width, global_height;
static void *bitmap_memory;
static HWND global_hwnd;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static uint32_t rgb_to_u32(uint8_t r, uint8_t g, uint8_t b) {
  // Credit: https://stackoverflow.com/a/39979191/8094047
  uint8_t alpha = 255;
  return (alpha << 24) + (r << 16) + (g << 8) + b;
}

void minimal_window_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t *pixel = (uint32_t *)bitmap_memory;
  pixel += y * global_width + x;
  *pixel = rgb_to_u32(r, g, b);
}

void minimal_window_create_fixed_size_window(int width, int height) {
  global_width = width;
  global_height = height;

  // Register the window class
  const wchar_t CLASS_NAME[] = L"Minimal Window Class";

  WNDCLASS wc = {0};
  HINSTANCE hInstance = GetModuleHandle(NULL);

  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);

  RegisterClass(&wc);

  // Create the window
  RECT window_rect = {0, 0, width, height};
  AdjustWindowRect(&window_rect, FIXED_SIZE_WINDOW_STYLE, FALSE);
  global_hwnd = CreateWindowEx(0,                       // Optional window styles.
                               CLASS_NAME,              // Window class
                               L"Minimal Window",       // Window text
                               FIXED_SIZE_WINDOW_STYLE, // Window style

                               // Size and position
                               CW_USEDEFAULT, CW_USEDEFAULT, window_rect.right - window_rect.left,
                               window_rect.bottom - window_rect.top,

                               NULL,      // Parent window
                               NULL,      // Menu
                               hInstance, // Instance handle
                               NULL       // Additional application data
  );

  if (global_hwnd == NULL) {
    return;
  }

  // Allocate memory for the bitmap
  int bytes_per_pixel = 4;
  bitmap_memory = malloc(width * height * bytes_per_pixel);

  // bitmap_info struct for StretchDIBits
  bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
  bitmap_info.bmiHeader.biWidth = width;
  // Negative height makes top left as the coordinate system origin for the DrawPixel function, otherwise its bottom
  // left
  bitmap_info.bmiHeader.biHeight = -height;
  bitmap_info.bmiHeader.biPlanes = 1;
  bitmap_info.bmiHeader.biBitCount = 32;
  bitmap_info.bmiHeader.biCompression = BI_RGB;

  ShowWindow(global_hwnd, SW_NORMAL);
}

bool minimal_window_process_events() {
  MSG msg = {0};
  if (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    return true;
  }
  free(bitmap_memory);
  return false;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  case WM_PAINT:
    // We must handle WM_PAINT like this, otherwise drawing outside event handler will not work
    // TODO: is this bad? :/
    return 0;

  default:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}

void minimal_window_request_update() {
  HDC hdc = GetDC(global_hwnd);
  StretchDIBits(hdc, 0, 0, global_width, global_height, 0, 0, global_width, global_height, bitmap_memory, &bitmap_info,
                DIB_RGB_COLORS, SRCCOPY);
  ReleaseDC(global_hwnd, hdc);
}
