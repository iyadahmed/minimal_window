#include "nanogui.h"

#include <iostream>


#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

#define FIXED_SIZE_WINDOW_STYLE (WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU)

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void nano_gui_create_fixed_size_window(int width, int height) {
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"NanoGUI Window Class";

    WNDCLASS wc = {};
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
            0,                              // Optional window styles.
            CLASS_NAME,                     // Window class
            L"NanoGUI Window",    // Window text
            FIXED_SIZE_WINDOW_STYLE,            // Window style

            // Size and position
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,

            nullptr,       // Parent window
            nullptr,       // Menu
            hInstance,  // Instance handle
            nullptr        // Additional application data
    );

    if (hwnd == nullptr) {
        return;
    }

    ShowWindow(hwnd, SW_NORMAL);
}

void nano_gui_run_main_loop() {
    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
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
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);
        }
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}
