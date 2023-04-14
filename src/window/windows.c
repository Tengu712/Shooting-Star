#include "../window.h"

#include <windows.h>
#include <stdlib.h>

static HINSTANCE g_hinst;
static HWND g_hwnd;

static LRESULT WINAPI WindowProcedure(HWND hwnd, uint32_t msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

void create_window_param(WindowParam *out) {
    out->win32.hinst = (void *)g_hinst;
    out->win32.hwnd = (void *)g_hwnd;
}

warn_t create_window(const char *title, uint32_t width, uint32_t height) {
    ss_info("initializing win32 window ...");
    ss_indent_logger();
    // instance handle
    g_hinst = GetModuleHandle(NULL);
    // window class
    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    const WNDCLASSEXW wc = {
        sizeof(WNDCLASSEXW),
        CS_CLASSDC,
        WindowProcedure,
        0L,
        0L,
        g_hinst,
        NULL,
        NULL,
        NULL,
        NULL,
        L"WIN32APIWINDOW",
        NULL,
    };
    if (!RegisterClassExW(&wc)) ss_error("failed to register window class.");
    // adjust window size
    RECT rect = { 0, 0, (long)width, (long)height };
    AdjustWindowRect(&rect, style, FALSE);
    // window
    wchar_t wtitle[256] = {};
    if (!MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, title, -1, wtitle, 256)) ss_error("failed to convert title multibyte to wide.");
    g_hwnd = CreateWindowExW(
        0,
        L"WIN32APIWINDOW",
        wtitle,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (int)(rect.right - rect.left),
        (int)(rect.bottom - rect.top),
        NULL,
        NULL,
        g_hinst,
        NULL
    );
    if (!g_hwnd) ss_error("failed to create window.");
    // finish
    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);
    ss_dedent_logger();
    ss_info("win32 window initialization succeeded.");
    return 0;
}

int32_t do_window_events(void) {
    MSG msg;
    while (1) {
        if (PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return 1;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }
        return 0;
    }
}

void terminate_window(void) {
    UnregisterClassW(L"WIN32APIWINDOW", g_hinst);
    DestroyWindow(g_hwnd);
}
