#include "nanogui.h"


#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

#define FIXED_SIZE_WINDOW_STYLE (WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU)


void *bitmap_memory;
BITMAPINFO bitmap_info;
int global_width, global_height;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void nano_gui_create_fixed_size_window(int width, int height) {
    global_width = width;
    global_height = height;

    // Register the window class.
    const wchar_t CLASS_NAME[] = L"NanoGUI Window Class";

    WNDCLASS wc = {};
    HINSTANCE hInstance = GetModuleHandle(NULL);

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
            0,                              // Optional window styles.
            CLASS_NAME,                     // Window class
            L"NanoGUI Window",    // Window text
            FIXED_SIZE_WINDOW_STYLE,            // Window style

            // Size and position
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,

            NULL,       // Parent window
            NULL,       // Menu
            hInstance,  // Instance handle
            NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return;
    }

    // Allocate memory for the bitmap
    int bytes_per_pixel = 4;
    bitmap_memory = VirtualAlloc(0,
                                 width * height * bytes_per_pixel,
                                 MEM_RESERVE | MEM_COMMIT,
                                 PAGE_READWRITE
    );

    // bitmap_info struct for StretchDIBits
    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = width;
    // Negative height makes top left as the coordinate system origin for the DrawPixel function, otherwise its bottom left
    bitmap_info.bmiHeader.biHeight = -height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    ShowWindow(hwnd, SW_NORMAL);
}

void nano_gui_run_main_loop() {
    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // All painting occurs here, between BeginPaint and EndPaint.
            StretchDIBits(hdc,
                          0, 0,
                          global_width, global_height,
                          0, 0,
                          global_width, global_height,
                          bitmap_memory, &bitmap_info,
                          DIB_RGB_COLORS, SRCCOPY);
            EndPaint(hwnd, &ps);
        }
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}
