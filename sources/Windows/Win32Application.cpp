#include "Win32Application.hpp"

namespace gerium::windows {

Win32Application::Win32Application(gerium_utf8_t title,
                                   gerium_uint32_t width,
                                   gerium_uint32_t height,
                                   gerium_application_mode_flags_t mode,
                                   HINSTANCE instance) :
    _hInstance(instance ? instance : GetModuleHandleW(nullptr)),
    _hWnd(nullptr),
    _running(false) {
    SetProcessDPIAware();

    WNDCLASSEXW wndClassEx;
    wndClassEx.cbSize        = sizeof(WNDCLASSEXW);
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

    if (!RegisterClassExW(&wndClassEx)) {
        throw Exception(GERIUM_STATE_NO_DISPLAY, "Failed to register window");
    }

    RECT rect;
    rect.left   = 0;
    rect.right  = (LONG) width;
    rect.top    = 0;
    rect.bottom = (LONG) height;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    auto wTitle = wideString(title);

    _hWnd = CreateWindowExW(WS_EX_TOPMOST,
                            _kClassName,
                            wTitle.c_str(),
                            WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            rect.right - rect.left,
                            rect.bottom - rect.top,
                            nullptr,
                            nullptr,
                            _hInstance,
                            nullptr);

    if (!_hWnd) {
        UnregisterClassW(_kClassName, _hInstance);
        throw Exception(GERIUM_STATE_NO_DISPLAY, "Failed to create window");
    }

    SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR) this);
}

gerium_runtime_platform_t Win32Application::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_WINDOWS;
}

void Win32Application::onRun() {
    if (!_hWnd) {
        throw Exception(GERIUM_STATE_APPLICATION_TERMINATED, "The application is already completed");
    }
    if (_running) {
        throw Exception(GERIUM_STATE_APPLICATION_RUNNING, "The application is already running");
    }
    _running = true;

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

    UnregisterClassW(_kClassName, _hInstance);
}

void Win32Application::onExit() noexcept {
    if (_hWnd) {
        DestroyWindow(_hWnd);
        _hWnd = nullptr;
    }
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

std::wstring Win32Application::wideString(gerium_utf8_t utf8) {
    auto byteCount = (int) utf8len(utf8);
    std::wstring result;
    result.resize(byteCount * 4 + 1);
    MultiByteToWideChar(CP_UTF8, 0, (LPCCH) utf8, byteCount, result.data(), (int) result.size());
    return result;
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
