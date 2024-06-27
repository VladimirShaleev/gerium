#include "Win32Application.hpp"

namespace gerium::windows {

Win32Application::Win32Application(gerium_utf8_t title,
                                   gerium_uint32_t width,
                                   gerium_uint32_t height,
                                   // gerium_application_mode_flags_t mode,
                                   HINSTANCE instance) :
    _hInstance(instance ? instance : GetModuleHandleW(nullptr)),
    _hWnd(nullptr),
    _running(false),
    _resizing(false),
    _visibility(false),
    _windowPlacement({}),
    _style(0),
    _styleEx(0) {
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
        throw Exception(GERIUM_RESULT_NO_DISPLAY, "Failed to register window");
    }

    RECT rect;
    rect.left   = 0;
    rect.right  = (LONG) width;
    rect.top    = 0;
    rect.bottom = (LONG) height;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    auto wTitle = wideString(title);

    _hWnd = CreateWindowExW(0,
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
        throw Exception(GERIUM_RESULT_NO_DISPLAY, "Failed to create window");
    }

    SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR) this);
}

gerium_runtime_platform_t Win32Application::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_WINDOWS;
}

void Win32Application::onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const {
    if (displays) {
        _monitors.clear();
        _modes.clear();
        _gpuNames.clear();
        _displayNames.clear();

        std::vector<DISPLAYCONFIG_PATH_INFO> paths;
        std::vector<DISPLAYCONFIG_MODE_INFO> modes;
        UINT32 flags = QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE;
        LONG result  = ERROR_SUCCESS;
        UINT32 pathCount;
        UINT32 modeCount;

        do {
            result = GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);

            if (result != ERROR_SUCCESS) {
                throw Exception(GERIUM_RESULT_NO_DISPLAY, "Get display config failed");
            }
            paths.resize(pathCount);
            modes.resize(modeCount);

            result = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);
            paths.resize(pathCount);
            modes.resize(modeCount);

        } while (result == ERROR_INSUFFICIENT_BUFFER);

        if (result != ERROR_SUCCESS) {
            throw Exception(GERIUM_RESULT_NO_DISPLAY, "Get display config failed");
        }

        for (auto& path : paths) {
            DISPLAYCONFIG_TARGET_DEVICE_NAME targetName{};
            targetName.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
            targetName.header.size      = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME);
            targetName.header.adapterId = path.targetInfo.adapterId;
            targetName.header.id        = path.targetInfo.id;

            if (DisplayConfigGetDeviceInfo(&targetName.header) != ERROR_SUCCESS) {
                throw Exception(GERIUM_RESULT_NO_DISPLAY, "Get display config failed");
            }

            DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName{};
            sourceName.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
            sourceName.header.size      = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME);
            sourceName.header.adapterId = path.sourceInfo.adapterId;
            sourceName.header.id        = path.sourceInfo.id;

            if (DisplayConfigGetDeviceInfo(&sourceName.header) != ERROR_SUCCESS) {
                throw Exception(GERIUM_RESULT_NO_DISPLAY, "Get display config failed");
            }

            _monitors[sourceName.viewGdiDeviceName] =
                utf8String(targetName.flags.friendlyNameFromEdid ? targetName.monitorFriendlyDeviceName : L"Unknown");
        }
    }

    gerium_uint32_t displayIndex = 0;

    DISPLAY_DEVICEW display{ sizeof(DISPLAY_DEVICEW) };
    for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &display, 0); ++i) {
        if (display.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
            if (displays) {
                if (displayIndex == displayCount) {
                    break;
                }
                gerium_display_info_t& info = displays[displayIndex++];

                if (auto it = _monitors.find(display.DeviceName); it != _monitors.end()) {
                    info.device_name = it->second.data();
                } else {
                    info.device_name = "Unknown";
                }

                _gpuNames.push_back(utf8String(display.DeviceString));
                info.gpu_name = _gpuNames.back().data();

                DISPLAY_DEVICEW displayName{ sizeof(DISPLAY_DEVICEW) };
                EnumDisplayDevicesW(display.DeviceName, 0, &displayName, 0);
                _displayNames.push_back(utf8String(displayName.DeviceString));
                info.name = _displayNames.back().data();

                info.mode_count = 0;
                DEVMODEW devMode{};
                for (DWORD m = 0; EnumDisplaySettingsW(display.DeviceName, m, &devMode); ++m) {
                    if (devMode.dmDisplayFrequency <= 1 || devMode.dmDisplayFrequency >= 30) {
                        ++info.mode_count;
                        _modes.emplace_back(
                            gerium_uint16_t(devMode.dmPelsWidth),
                            gerium_uint16_t(devMode.dmPelsHeight),
                            gerium_uint16_t(devMode.dmDisplayFrequency == 1 ? 0 : devMode.dmDisplayFrequency));
                    }
                }
                info.modes = _modes.data() + _modes.size() - info.mode_count;
            } else {
                ++displayIndex;
            }
        }
    }
    displayCount = displayIndex;
}

bool Win32Application::onGetFullscreen() const noexcept {
    return _style != 0;
}

BOOL monitorEnum(HMONITOR hMonitor, HDC, LPRECT, LPARAM) {
    MONITORINFOEX info{ sizeof(MONITORINFOEX) };
    GetMonitorInfoW(hMonitor, &info);

    return true;
}

void Win32Application::onSetFullscreen(bool fullscreen) noexcept {
    if (fullscreen) {
        std::vector<DISPLAYCONFIG_PATH_INFO> paths;
        std::vector<DISPLAYCONFIG_MODE_INFO> modes;
        UINT32 flags = QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE;
        LONG result  = ERROR_SUCCESS;

        do {
            // Determine how many path and mode structures to allocate
            UINT32 pathCount, modeCount;
            result = GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);

            if (result != ERROR_SUCCESS) {
                // return HRESULT_FROM_WIN32(result);
            }

            // Allocate the path and mode arrays
            paths.resize(pathCount);
            modes.resize(modeCount);

            // Get all active paths and their modes
            result = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);

            // The function may have returned fewer paths/modes than estimated
            paths.resize(pathCount);
            modes.resize(modeCount);

            // It's possible that between the call to GetDisplayConfigBufferSizes and QueryDisplayConfig
            // that the display state changed, so loop on the case of ERROR_INSUFFICIENT_BUFFER.
        } while (result == ERROR_INSUFFICIENT_BUFFER);

        if (result != ERROR_SUCCESS) {
            // return HRESULT_FROM_WIN32(result);
        }

        // For each active path
        for (auto& path : paths) {
            // Find the target (monitor) friendly name
            DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
            targetName.header.adapterId                 = path.targetInfo.adapterId;
            targetName.header.id                        = path.targetInfo.id;
            targetName.header.type                      = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
            targetName.header.size                      = sizeof(targetName);
            result                                      = DisplayConfigGetDeviceInfo(&targetName.header);

            if (result != ERROR_SUCCESS) {
                // return HRESULT_FROM_WIN32(result);
            }

            // Find the adapter device name
            DISPLAYCONFIG_ADAPTER_NAME adapterName = {};
            adapterName.header.adapterId           = path.targetInfo.adapterId;
            adapterName.header.type                = DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME;
            adapterName.header.size                = sizeof(adapterName);

            result = DisplayConfigGetDeviceInfo(&adapterName.header);

            if (result != ERROR_SUCCESS) {
                // return HRESULT_FROM_WIN32(result);
            }

            DISPLAYCONFIG_SOURCE_DEVICE_NAME soruceName = {};
            soruceName.header.adapterId                 = path.sourceInfo.adapterId;
            soruceName.header.id                        = path.sourceInfo.id;
            soruceName.header.type                      = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
            soruceName.header.size                      = sizeof(soruceName);

            result = DisplayConfigGetDeviceInfo(&soruceName.header);

            if (result != ERROR_SUCCESS) {
                // return HRESULT_FROM_WIN32(result);
            }

            std::wostringstream ss;
            ss << L"Monitor with name "
               << (targetName.flags.friendlyNameFromEdid ? targetName.monitorFriendlyDeviceName : L"Unknown")
               << L" is connected to adapter " << adapterName.adapterDevicePath << L" on target " << path.targetInfo.id
               << L"\n";

            auto a = ss.str();
            auto b = a;
        }

        DISPLAY_DEVICEW display{ sizeof(DISPLAY_DEVICEW) };
        for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &display, 0); ++i) {
            // DISPLAY_DEVICEW displayName{ sizeof(DISPLAY_DEVICEW) };
            // EnumDisplayDevicesW(display.DeviceName, 0, &displayName, 0);

            // GetMonitorInfo(MonitorFromWindow(_hWnd, MONITOR_DEFAULTTOPRIMARY))

            if (display.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
                DEVMODEW devMode{};
                for (DWORD mode = 0; EnumDisplaySettingsW(display.DeviceName, mode, &devMode); ++mode) {
                    auto a = devMode;
                }
            }
        }

        saveWindowPlacement();
        SetWindowLong(_hWnd, GWL_STYLE, WS_POPUP);
        SetWindowLong(_hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
        ShowWindow(_hWnd, SW_SHOWMAXIMIZED);
    } else {
        restoreWindowPlacement();
    }
}

void Win32Application::onRun() {
    if (!_hWnd) {
        throw Exception(GERIUM_RESULT_APPLICATION_TERMINATED, "The application is already completed");
    }
    if (_running) {
        throw Exception(GERIUM_RESULT_APPLICATION_RUNNING, "The application is already running");
    }
    _running = true;

    if (!callStateFunc(GERIUM_APPLICATION_STATE_CREATE) || !callStateFunc(GERIUM_APPLICATION_STATE_INITIALIZE)) {
        throw Exception(GERIUM_RESULT_APPLICATION_TERMINATED, "The callback requested application termination");
    }

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

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    UnregisterClassW(_kClassName, _hInstance);
}

void Win32Application::onExit() noexcept {
    if (_hWnd) {
        DestroyWindow(_hWnd);
    }
}

LRESULT Win32Application::wndProc(UINT message, WPARAM wParam, LPARAM lParam) {
    auto prevVisibility = _visibility;
    switch (message) {
        case WM_SYSKEYUP:
            if (wParam == VK_RETURN) {
                gerium_application_set_fullscreen(this, !gerium_application_get_fullscreen(this));
            }
            break;

        case WM_KEYUP:
            break;

        case WM_CLOSE:
            callStateFunc(GERIUM_APPLICATION_STATE_INVISIBLE);
            exit();
            break;

        case WM_ACTIVATEAPP:
            callStateFunc(wParam ? GERIUM_APPLICATION_STATE_GOT_FOCUS : GERIUM_APPLICATION_STATE_LOST_FOCUS);
            break;

        case WM_SIZING:
            _resizing = true;
            return TRUE;

        case WM_SIZE:
            _visibility = true;
            switch (wParam) {
                case SIZE_RESTORED:
                    if (_resizing) {
                        callStateFunc(GERIUM_APPLICATION_STATE_RESIZE);
                    } else {
                        callStateFunc(GERIUM_APPLICATION_STATE_NORMAL);
                    }
                    break;
                case SIZE_MINIMIZED:
                    callStateFunc(GERIUM_APPLICATION_STATE_MINIMIZE);
                    _visibility = false;
                    break;
                case SIZE_MAXIMIZED:
                    callStateFunc(onGetFullscreen() ? GERIUM_APPLICATION_STATE_FULLSCREEN
                                                    : GERIUM_APPLICATION_STATE_MAXIMIZE);
                    break;
            }
            if (_visibility != prevVisibility) {
                callStateFunc(_visibility ? GERIUM_APPLICATION_STATE_VISIBLE : GERIUM_APPLICATION_STATE_INVISIBLE);
            }
            _resizing = false;
            break;

        case WM_DESTROY:
            callStateFunc(GERIUM_APPLICATION_STATE_UNINITIALIZE);
            callStateFunc(GERIUM_APPLICATION_STATE_DESTROY);
            PostQuitMessage(0);
            _hWnd    = nullptr;
            _running = false;
            break;

        default:
            return DefWindowProc(_hWnd, message, wParam, lParam);
    }
    return 0;
}

void Win32Application::saveWindowPlacement() {
    GetWindowPlacement(_hWnd, &_windowPlacement);
    _style   = GetWindowLong(_hWnd, GWL_STYLE);
    _styleEx = GetWindowLong(_hWnd, GWL_EXSTYLE);
}

void Win32Application::restoreWindowPlacement() {
    SetWindowLong(_hWnd, GWL_STYLE, _style);
    SetWindowLong(_hWnd, GWL_EXSTYLE, _styleEx);
    ShowWindow(_hWnd, SW_SHOWNORMAL);
    SetWindowPlacement(_hWnd, &_windowPlacement);
    _style   = 0;
    _styleEx = 0;
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

std::string Win32Application::utf8String(const std::wstring& wstr) {
    std::string result;
    result.resize(wstr.length() * 2);
    WideCharToMultiByte(
        CP_UTF8, 0, (LPCWCH) wstr.data(), wstr.length(), result.data(), (int) result.size(), nullptr, nullptr);
    return result;
}

LRESULT Win32Application::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    auto application = (Win32Application*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    return application ? application->wndProc(message, wParam, lParam) : DefWindowProc(hWnd, message, wParam, lParam);
}

} // namespace gerium::windows

gerium_result_t gerium_windows_application_create(gerium_utf8_t title,
                                                  gerium_uint32_t width,
                                                  gerium_uint32_t height,
                                                  // gerium_application_mode_flags_t mode,
                                                  HINSTANCE instance,
                                                  gerium_application_t* application) {
    using namespace gerium;
    using namespace gerium::windows;
    return Object::create<Win32Application>(*application, title, width, height, /* mode, */ instance);
}
