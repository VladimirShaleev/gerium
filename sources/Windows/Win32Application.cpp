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
    _styleEx(0),
    _prevState(GERIUM_APPLICATION_STATE_UNKNOWN) {
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
        throw Exception(GERIUM_RESULT_ERROR_NO_DISPLAY, "Failed to register window");
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
        throw Exception(GERIUM_RESULT_ERROR_NO_DISPLAY, "Failed to create window");
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
                throw Exception(GERIUM_RESULT_ERROR_NO_DISPLAY, "Get display config failed");
            }
            paths.resize(pathCount);
            modes.resize(modeCount);

            result = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);
            paths.resize(pathCount);
            modes.resize(modeCount);

        } while (result == ERROR_INSUFFICIENT_BUFFER);

        if (result != ERROR_SUCCESS) {
            throw Exception(GERIUM_RESULT_ERROR_NO_DISPLAY, "Get display config failed");
        }

        for (auto& path : paths) {
            DISPLAYCONFIG_TARGET_DEVICE_NAME targetName{};
            targetName.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
            targetName.header.size      = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME);
            targetName.header.adapterId = path.targetInfo.adapterId;
            targetName.header.id        = path.targetInfo.id;

            if (DisplayConfigGetDeviceInfo(&targetName.header) != ERROR_SUCCESS) {
                throw Exception(GERIUM_RESULT_ERROR_NO_DISPLAY, "Get display config failed");
            }

            DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName{};
            sourceName.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
            sourceName.header.size      = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME);
            sourceName.header.adapterId = path.sourceInfo.adapterId;
            sourceName.header.id        = path.sourceInfo.id;

            if (DisplayConfigGetDeviceInfo(&sourceName.header) != ERROR_SUCCESS) {
                throw Exception(GERIUM_RESULT_ERROR_NO_DISPLAY, "Get display config failed");
            }

            _monitors[sourceName.viewGdiDeviceName] =
                utf8String(targetName.flags.friendlyNameFromEdid ? targetName.monitorFriendlyDeviceName : L"Unknown");
        }
    }

    gerium_uint32_t displayIndex = 0;
    enumDisplays(displayCount, displayIndex, true, displays);
    enumDisplays(displayCount, displayIndex, false, displays);
    displayCount = displayIndex;

    if (displays) {
        for (gerium_uint32_t i = 0, offset = 0; i < displayIndex; ++i) {
            displays[i].gpu_name = _displayNames[i * 2].data();
            displays[i].name     = _displayNames[i * 2 + 1].data();
            displays[i].modes    = _modes.data() + offset;
            offset += displays[i].mode_count;
        }
    }
}

bool Win32Application::onIsFullscreen() const noexcept {
    return _style != 0;
}

void Win32Application::onFullscreen(bool fullscreen, const gerium_display_mode_t* mode) {
    if (fullscreen) {
        saveWindowPlacement();

        DEVMODE currentMode{};
        if (!EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &currentMode)) {
            throw Exception(GERIUM_RESULT_ERROR_CHANGE_DISPLAY_MODE, "Error change display mode");
        }

        auto newMode = currentMode;
        if (mode) {
            newMode.dmPelsWidth  = mode->width;
            newMode.dmPelsHeight = mode->height;
            newMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;
            if (mode->refresh_rate) {
                newMode.dmDisplayFrequency = mode->refresh_rate;
                newMode.dmFields |= DM_DISPLAYFREQUENCY;
            }
        }

        if (ChangeDisplaySettings(&newMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            if (ChangeDisplaySettings(&currentMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
                throw Exception(GERIUM_RESULT_ERROR_CHANGE_DISPLAY_MODE, "Error change display mode");
            }
        }

        SetWindowLong(_hWnd, GWL_STYLE, WS_POPUP);
        SetWindowLong(_hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
        if (_running) {
            ShowWindow(_hWnd, SW_SHOWMAXIMIZED);
        }
    } else {
        ChangeDisplaySettings(nullptr, 0);
        restoreWindowPlacement();
    }
}

void Win32Application::onRun() {
    if (!_hWnd) {
        throw Exception(GERIUM_RESULT_ERROR_APPLICATION_TERMINATED, "The application is already completed");
    }
    if (_running) {
        throw Exception(GERIUM_RESULT_ERROR_APPLICATION_RUNNING, "The application is already running");
    }
    _running = true;

    if (!changeState(GERIUM_APPLICATION_STATE_CREATE) || !changeState(GERIUM_APPLICATION_STATE_INITIALIZE)) {
        throw Exception(GERIUM_RESULT_ERROR_APPLICATION_TERMINATED, "The callback requested application termination");
    }

    ShowWindow(_hWnd, onIsFullscreen() ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
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
        case WM_SYSKEYUP: // TODO: Need it for development and testing. Remove later
            if (wParam == VK_RETURN) {
                gerium_application_fullscreen(this, !gerium_application_is_fullscreen(this), nullptr);
            }
            break;

        case WM_KEYUP: // TODO: Need it for development and testing. Remove later
            if (wParam == VK_ESCAPE) {
                gerium_application_exit(this);
            }
            break;

        case WM_CLOSE:
            changeState(GERIUM_APPLICATION_STATE_INVISIBLE);
            exit();
            break;

        case WM_ACTIVATEAPP:
            changeState(wParam ? GERIUM_APPLICATION_STATE_GOT_FOCUS : GERIUM_APPLICATION_STATE_LOST_FOCUS);
            break;

        case WM_SIZING:
            _resizing = true;
            return TRUE;

        case WM_SIZE:
            _visibility = true;
            switch (wParam) {
                case SIZE_RESTORED:
                    if (_resizing) {
                        changeState(GERIUM_APPLICATION_STATE_RESIZE);
                    } else {
                        changeState(GERIUM_APPLICATION_STATE_NORMAL);
                    }
                    break;
                case SIZE_MINIMIZED:
                    changeState(GERIUM_APPLICATION_STATE_MINIMIZE);
                    _visibility = false;
                    break;
                case SIZE_MAXIMIZED:
                    changeState(onIsFullscreen() ? GERIUM_APPLICATION_STATE_FULLSCREEN
                                                 : GERIUM_APPLICATION_STATE_MAXIMIZE);
                    break;
            }
            if (_visibility != prevVisibility) {
                changeState(_visibility ? GERIUM_APPLICATION_STATE_VISIBLE : GERIUM_APPLICATION_STATE_INVISIBLE);
            }
            _resizing = false;
            break;

        case WM_EXITSIZEMOVE:
            if (_prevState == GERIUM_APPLICATION_STATE_RESIZE) {
                changeState(GERIUM_APPLICATION_STATE_RESIZED);
            }
            break;

        case WM_DESTROY:
            changeState(GERIUM_APPLICATION_STATE_UNINITIALIZE);
            changeState(GERIUM_APPLICATION_STATE_DESTROY);
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
    if (_running) {
        ShowWindow(_hWnd, SW_SHOWNORMAL);
    }
    SetWindowPlacement(_hWnd, &_windowPlacement);
    _style   = 0;
    _styleEx = 0;
}

bool Win32Application::changeState(gerium_application_state_t newState) noexcept {
    if (_prevState != newState || newState == GERIUM_APPLICATION_STATE_RESIZE) {
        _prevState = newState;
        return callStateFunc(newState);
    }
    return true;
}

void Win32Application::enumDisplays(gerium_uint32_t displayCount,
                                    gerium_uint32_t& displayIndex,
                                    bool primary,
                                    gerium_display_info_t* displays) const {
    DISPLAY_DEVICEW display{ sizeof(DISPLAY_DEVICEW) };
    for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &display, 0); ++i) {
        if (display.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
            bool displayPrimary = display.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE;
            if (displayPrimary != primary) {
                continue;
            }
            if (displays) {
                if (displayIndex == displayCount) {
                    break;
                }

                gerium_display_info_t& info = displays[displayIndex++];
                info.mode_count             = 0;

                if (auto it = _monitors.find(display.DeviceName); it != _monitors.end()) {
                    info.device_name = it->second.data();
                } else {
                    info.device_name = "Unknown";
                }

                DISPLAY_DEVICEW displayName{ sizeof(DISPLAY_DEVICEW) };
                EnumDisplayDevicesW(display.DeviceName, 0, &displayName, 0);

                _displayNames.push_back(utf8String(display.DeviceString));
                _displayNames.push_back(utf8String(displayName.DeviceString));

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
            } else {
                ++displayIndex;
            }
        }
    }
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
