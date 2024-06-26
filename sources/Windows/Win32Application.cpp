#include "Win32Application.hpp"

namespace gerium::windows {

Win32Application::Win32Application(gerium_utf8_t title,
                                   gerium_uint32_t width,
                                   gerium_uint32_t height,
                                   gerium_application_mode_flags_t mode,
                                   HINSTANCE instance) :
    _hInstance(instance ? instance : GetModuleHandle(nullptr)),
    _hWnd(nullptr),
    _title(title) {
    SetProcessDPIAware();
}

gerium_runtime_platform_t Win32Application::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_WINDOWS;
}

gerium_state_t Win32Application::onRun() noexcept {
    WNDCLASSEX wndClassEx;
    wndClassEx.cbSize        = sizeof(WNDCLASSEX);
    wndClassEx.style         = 0;
    wndClassEx.lpfnWndProc   = wndProc;
    wndClassEx.cbClsExtra    = 0;
    wndClassEx.cbWndExtra    = sizeof(this);
    wndClassEx.hInstance     = _hInstance;
    wndClassEx.hIcon         = nullptr;
    wndClassEx.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wndClassEx.hbrBackground = nullptr;
    wndClassEx.lpszMenuName  = nullptr;
    wndClassEx.lpszClassName = _kClassName;
    wndClassEx.hIconSm       = nullptr;

    if (!RegisterClassEx(&wndClassEx)) {
        return GERIUM_STATE_NO_DISPLAY;
    }

    RECT rect;
    rect.left   = 0;
    rect.right  = 800;
    rect.top    = 0;
    rect.bottom = 600;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE);

    _hWnd = CreateWindowEx(WS_EX_TOPMOST,
                           _kClassName,
                           _title.c_str(),
                           WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           rect.right - rect.left,
                           rect.bottom - rect.top,
                           nullptr,
                           nullptr,
                           _hInstance,
                           nullptr);

    if (!_hWnd) {
        UnregisterClass(_kClassName, _hInstance);
        return GERIUM_STATE_NO_DISPLAY;
    }

    SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR) this);

    ShowWindow(_hWnd, SW_SHOWNORMAL);
    UpdateWindow(_hWnd);

    MSG msg{};
    while (true) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE)) {
            if (!GetMessage(&msg, nullptr, 0, 0)) {
                break;
            }

            if (!waitInBackground(&msg)) {
                break;
            }

            DispatchMessage(&msg);
        }
    }

    return GERIUM_STATE_SUCCESS;
}

void Win32Application::onExit() noexcept {
}

LRESULT Win32Application::wndProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        default:
            return DefWindowProc(_hWnd, message, wParam, lParam);
    }
    return 0;
}

bool Win32Application::waitInBackground(LPMSG pMsg) {
    if (GetActiveWindow()) {
        return true;
    }

    while (GetMessage(pMsg, nullptr, 0, 0)) {
        DispatchMessage(pMsg);
        if (GetActiveWindow()) {
            return true;
        }
    }
    return false;
}

LRESULT Win32Application::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    auto application = (Win32Application*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return application ? application->wndProc(message, wParam, lParam)
                               : DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

} // namespace gerium::windows

gerium_state_t gerium_windows_application_create(gerium_utf8_t title,
                                                 gerium_uint32_t width,
                                                 gerium_uint32_t height,
                                                 gerium_application_mode_flags_t mode,
                                                 HINSTANCE instance,
                                                 gerium_application_t* application) {
    using namespace gerium;
    using namespace gerium::windows;
    return Object::create<Win32Application>(*application, title, width, height, mode, instance);
}
