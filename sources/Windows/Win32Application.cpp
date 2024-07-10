#include "Win32Application.hpp"

#include <imgui_impl_win32.h>

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace gerium::windows {

Win32Application::Win32Application(gerium_utf8_t title,
                                   gerium_uint32_t width,
                                   gerium_uint32_t height,
                                   HINSTANCE instance) :
    _hInstance(instance ? instance : GetModuleHandleW(nullptr)),
    _hWnd(nullptr),
    _running(false),
    _resizing(false),
    _visibility(false),
    _windowPlacement({}),
    _styleEx(0),
    _styleFlags(GERIUM_APPLICATION_STYLE_RESIZABLE_BIT | GERIUM_APPLICATION_STYLE_MINIMIZABLE_BIT |
                GERIUM_APPLICATION_STYLE_MAXIMIZABLE_BIT),
    _minWidth(0),
    _minHeight(0),
    _maxWidth(0),
    _maxHeight(0),
    _newWidth(std::numeric_limits<gerium_uint16_t>::max()),
    _newHeight(std::numeric_limits<gerium_uint16_t>::max()) {
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
        error(GERIUM_RESULT_ERROR_NO_DISPLAY);
    }

    const auto [winWidth, winHeight] = clientSizeToWindowSize(width, height);

    const auto wTitle = wideString(title ? title : "");

    _hWnd = CreateWindowExW(0,
                            _kClassName,
                            wTitle.c_str(),
                            getStyle(),
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            winWidth,
                            winHeight,
                            nullptr,
                            nullptr,
                            _hInstance,
                            nullptr);

    if (!_hWnd) {
        UnregisterClassW(_kClassName, _hInstance);
        error(GERIUM_RESULT_ERROR_NO_DISPLAY);
    }

    SetWindowLongPtrW(_hWnd, GWLP_USERDATA, (LONG_PTR) this);
}

HINSTANCE Win32Application::hInstance() const noexcept {
    return _hInstance;
}

HWND Win32Application::hWnd() const noexcept {
    return _hWnd;
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
                error(GERIUM_RESULT_ERROR_NO_DISPLAY);
            }
            paths.resize(pathCount);
            modes.resize(modeCount);

            result = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);
            paths.resize(pathCount);
            modes.resize(modeCount);

        } while (result == ERROR_INSUFFICIENT_BUFFER);

        if (result != ERROR_SUCCESS) {
            error(GERIUM_RESULT_ERROR_NO_DISPLAY);
        }

        for (auto& path : paths) {
            DISPLAYCONFIG_TARGET_DEVICE_NAME targetName{};
            targetName.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
            targetName.header.size      = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME);
            targetName.header.adapterId = path.targetInfo.adapterId;
            targetName.header.id        = path.targetInfo.id;

            if (DisplayConfigGetDeviceInfo(&targetName.header) != ERROR_SUCCESS) {
                error(GERIUM_RESULT_ERROR_NO_DISPLAY);
            }

            DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName{};
            sourceName.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
            sourceName.header.size      = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME);
            sourceName.header.adapterId = path.sourceInfo.adapterId;
            sourceName.header.id        = path.sourceInfo.id;

            if (DisplayConfigGetDeviceInfo(&sourceName.header) != ERROR_SUCCESS) {
                error(GERIUM_RESULT_ERROR_NO_DISPLAY);
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
    return _styleEx != 0;
}

void Win32Application::onFullscreen(bool fullscreen, gerium_uint32_t displayId, const gerium_display_mode_t* mode) {
    if (fullscreen) {
        saveWindowPlacement();

        DEVMODEW currentMode{};
        if (!EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &currentMode)) {
            error(GERIUM_RESULT_ERROR_CHANGE_DISPLAY_MODE);
        }

        auto newMode = currentMode;
        if (displayId != std::numeric_limits<gerium_uint32_t>::max()) {
            DISPLAY_DEVICEW display{ sizeof(DISPLAY_DEVICEW) };
            if (EnumDisplayDevicesW(nullptr, displayId, &display, 0)) {
                if (display.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
                    DEVMODEW devMode{};
                    for (DWORD m = 0; EnumDisplaySettingsW(display.DeviceName, m, &devMode); ++m) {
                        if (devMode.dmDisplayFrequency <= 1 || devMode.dmDisplayFrequency >= 30) {
                            if (mode) {
                                auto width  = gerium_uint16_t(devMode.dmPelsWidth);
                                auto height = gerium_uint16_t(devMode.dmPelsHeight);
                                auto refreshRate =
                                    gerium_uint16_t(devMode.dmDisplayFrequency == 1 ? 0 : devMode.dmDisplayFrequency);

                                if (width == mode->width && height == mode->height &&
                                    mode->refresh_rate == refreshRate) {
                                    newMode          = devMode;
                                    newMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
                                    break;
                                }
                            } else {
                                newMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
                                break;
                            }
                        }
                    }
                }
            }
        } else if (mode) {
            newMode.dmPelsWidth  = mode->width;
            newMode.dmPelsHeight = mode->height;
            newMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;
            if (mode->refresh_rate) {
                newMode.dmDisplayFrequency = mode->refresh_rate;
                newMode.dmFields |= DM_DISPLAYFREQUENCY;
            }
        }

        if (ChangeDisplaySettingsW(&newMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            if (ChangeDisplaySettingsW(&currentMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
                error(GERIUM_RESULT_ERROR_CHANGE_DISPLAY_MODE);
            }
        }

        SetWindowLongW(_hWnd, GWL_STYLE, WS_POPUP);
        SetWindowLongW(_hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
        if (_running) {
            ShowWindow(_hWnd, SW_SHOWMAXIMIZED);
        }
    } else {
        ChangeDisplaySettingsW(nullptr, 0);
        restoreWindowPlacement();
        onSetSize(_newWidth, _newHeight);
    }
}

gerium_application_style_flags_t Win32Application::onGetStyle() const noexcept {
    return _styleFlags;
}

void Win32Application::onSetStyle(gerium_application_style_flags_t style) noexcept {
    _styleFlags = style;
    if (!onIsFullscreen()) {
        SetWindowLongW(_hWnd, GWL_STYLE, getStyle());
        ShowWindow(_hWnd, SW_SHOWNORMAL);
    }
}

void Win32Application::onGetMinSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    if (width) {
        *width = _minWidth;
    }
    if (height) {
        *height = _minHeight;
    }
}

void Win32Application::onGetMaxSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    if (width) {
        *width = _maxWidth;
    }
    if (height) {
        *height = _maxHeight;
    }
}

void Win32Application::onGetSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    const auto [w, h] = clientSize();
    if (width) {
        *width = w;
    }
    if (height) {
        *height = h;
    }
}

void Win32Application::onSetMinSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    _minWidth  = width;
    _minHeight = height;

    if (!onIsFullscreen()) {
        const auto [currentWidth, currentHeight] = clientSize();
        if (currentWidth < _minWidth || currentHeight < _minHeight) {
            _minWidth  = currentWidth < _minWidth ? _minWidth : currentWidth;
            _minHeight = currentHeight < _minHeight ? _minHeight : currentHeight;

            const auto [winWidth, winHeight] = clientSizeToWindowSize(_minWidth, _minHeight);
            SetWindowPos(_hWnd, nullptr, 0, 0, winWidth, winHeight, SWP_NOMOVE);
        }
    }
}

void Win32Application::onSetMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    _maxWidth  = width;
    _maxHeight = height;

    if (!onIsFullscreen()) {
        const auto [currentWidth, currentHeight] = clientSize();
        if (currentWidth > _maxWidth || currentHeight > _maxHeight) {
            _maxWidth  = currentWidth > _maxWidth ? _maxWidth : currentWidth;
            _maxHeight = currentHeight > _maxHeight ? _maxHeight : currentHeight;

            const auto [winWidth, winHeight] = clientSizeToWindowSize(_maxWidth, _maxHeight);
            SetWindowPos(_hWnd, nullptr, 0, 0, winWidth, winHeight, SWP_NOMOVE);
        }
    }
}

void Win32Application::onSetSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    constexpr auto maxValue = std::numeric_limits<gerium_uint16_t>::max();

    _newWidth  = width;
    _newHeight = height;

    if (!onIsFullscreen() && _newWidth != maxValue && _newHeight != maxValue) {
        const auto [winWidth, winHeight] = clientSizeToWindowSize(_newWidth, _newHeight);
        SetWindowPos(_hWnd, nullptr, 0, 0, winWidth, winHeight, SWP_NOMOVE);
        _newWidth  = maxValue;
        _newHeight = maxValue;
    }
}

gerium_utf8_t Win32Application::onGetTitle() const noexcept {
    const auto len = GetWindowTextLengthW(_hWnd);
    std::wstring title;
    title.resize(len + 1);
    GetWindowTextW(_hWnd, title.data(), len + 1);
    _title = utf8String(title);
    return _title.data();
}

void Win32Application::onSetTitle(gerium_utf8_t title) noexcept {
    const auto newTitle = wideString(title);
    SetWindowTextW(_hWnd, newTitle.data());
}

void Win32Application::onRun() {
    if (!_hWnd) {
        error(GERIUM_RESULT_ERROR_APPLICATION_TERMINATED);
    }
    if (_running) {
        error(GERIUM_RESULT_ERROR_APPLICATION_ALREADY_RUNNING);
    }
    _running = true;

    changeState(GERIUM_APPLICATION_STATE_CREATE);
    changeState(GERIUM_APPLICATION_STATE_INITIALIZE);

    ShowWindow(_hWnd, onIsFullscreen() ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
    UpdateWindow(_hWnd);

    LARGE_INTEGER prevTime;
    LARGE_INTEGER currentTime;
    LARGE_INTEGER microseconds;
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&prevTime);

    MSG msg{};
    while (msg.message != WM_QUIT) {
        if (callbackStateFailed()) {
            error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
        }

        if (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE)) {
            if (!GetMessage(&msg, nullptr, 0, 0)) {
                break;
            }

            if (getBackgroundWait() && !waitInBackground(&msg)) {
                break;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        QueryPerformanceCounter(&currentTime);
        microseconds.QuadPart = currentTime.QuadPart - prevTime.QuadPart;

        microseconds.QuadPart *= 1'000'000;
        microseconds.QuadPart /= frequency.QuadPart;

        auto elapsedMs = float(microseconds.QuadPart * 0.001);

        if (elapsedMs == 0.0f) {
            continue;
        }
        prevTime = currentTime;

        if (_running && !callFrameFunc(elapsedMs)) {
            error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
        }
    }

    UnregisterClassW(_kClassName, _hInstance);
}

void Win32Application::onExit() noexcept {
    if (_hWnd) {
        DestroyWindow(_hWnd);
    }
}

bool Win32Application::onIsRunning() const noexcept {
    return _running;
}

void Win32Application::onInitImGui() {
    ImGui_ImplWin32_Init((void*) _hWnd);
}

void Win32Application::onShutdownImGui() {
    ImGui_ImplWin32_Shutdown();
}

void Win32Application::onNewFrameImGui() {
    ImGui_ImplWin32_NewFrame();
}

LRESULT Win32Application::wndProc(UINT message, WPARAM wParam, LPARAM lParam) noexcept {
    if (ImGui_ImplWin32_WndProcHandler(_hWnd, message, wParam, lParam)) {
        return 1;
    }

    auto prevVisibility = _visibility;
    switch (message) {
        case WM_SYSKEYUP: // TODO: Need it for development and testing. Remove later
            if (wParam == VK_RETURN) {
                gerium_application_fullscreen(this,
                                              !gerium_application_is_fullscreen(this),
                                              std::numeric_limits<gerium_uint32_t>::max(),
                                              nullptr);
            }
            break;

        case WM_KEYUP: // TODO: Need it for development and testing. Remove later
            if (wParam == VK_ESCAPE) {
                gerium_application_exit(this);
            }
            break;

        case WM_CLOSE:
            changeState(GERIUM_APPLICATION_STATE_INVISIBLE, true);
            exit();
            break;

        case WM_ACTIVATEAPP:
            changeState(wParam ? GERIUM_APPLICATION_STATE_GOT_FOCUS : GERIUM_APPLICATION_STATE_LOST_FOCUS, true);
            break;

        case WM_SIZING:
            _resizing = true;
            return TRUE;

        case WM_SIZE:
            _visibility = true;
            switch (wParam) {
                case SIZE_RESTORED:
                    if (_resizing) {
                        changeState(GERIUM_APPLICATION_STATE_RESIZE, true);
                    } else {
                        changeState(GERIUM_APPLICATION_STATE_NORMAL, true);
                    }
                    break;
                case SIZE_MINIMIZED:
                    changeState(GERIUM_APPLICATION_STATE_MINIMIZE);
                    _visibility = false;
                    break;
                case SIZE_MAXIMIZED:
                    changeState(onIsFullscreen() ? GERIUM_APPLICATION_STATE_FULLSCREEN
                                                 : GERIUM_APPLICATION_STATE_MAXIMIZE,
                                true);
                    break;
            }
            if (_visibility != prevVisibility) {
                changeState(_visibility ? GERIUM_APPLICATION_STATE_VISIBLE : GERIUM_APPLICATION_STATE_INVISIBLE, true);
            }
            _resizing = false;
            break;

        case WM_EXITSIZEMOVE:
            if (currentState() == GERIUM_APPLICATION_STATE_RESIZE) {
                changeState(GERIUM_APPLICATION_STATE_RESIZED, true);
            }
            break;

        case WM_GETMINMAXINFO:
            if (!onIsFullscreen()) {
                auto minMaxInfo = (LPMINMAXINFO) lParam;

                const auto [minWinWidth, minWinHeight] = clientSizeToWindowSize(_minWidth, _minHeight);
                const auto [maxWinWidth, maxWinHeight] = clientSizeToWindowSize(_maxWidth, _maxHeight);

                minMaxInfo->ptMinTrackSize.x = minWinWidth;
                minMaxInfo->ptMinTrackSize.y = minWinHeight;
                minMaxInfo->ptMaxTrackSize.x = _maxWidth ? maxWinWidth : minMaxInfo->ptMaxSize.x;
                minMaxInfo->ptMaxTrackSize.y = _maxHeight ? maxWinHeight : minMaxInfo->ptMaxSize.y;
            }
            break;

        case WM_DESTROY:
            changeState(GERIUM_APPLICATION_STATE_UNINITIALIZE, true);
            changeState(GERIUM_APPLICATION_STATE_DESTROY, true);
            PostQuitMessage(0);
            _hWnd    = nullptr;
            _running = false;
            break;

        default:
            return DefWindowProcW(_hWnd, message, wParam, lParam);
    }
    return 0;
}

void Win32Application::saveWindowPlacement() {
    GetWindowPlacement(_hWnd, &_windowPlacement);
    _styleEx = GetWindowLongW(_hWnd, GWL_EXSTYLE);
}

void Win32Application::restoreWindowPlacement() {
    SetWindowLongW(_hWnd, GWL_STYLE, getStyle());
    SetWindowLongW(_hWnd, GWL_EXSTYLE, _styleEx);
    if (_running) {
        ShowWindow(_hWnd, SW_SHOWNORMAL);
    }
    SetWindowPlacement(_hWnd, &_windowPlacement);
    _styleEx = 0;
}

gerium_uint64_t Win32Application::getCurrentTime() noexcept {
    return gerium_uint64_t();
}

LONG Win32Application::getStyle() const noexcept {
    LONG result = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    if (_styleFlags & GERIUM_APPLICATION_STYLE_RESIZABLE_BIT) {
        result |= WS_THICKFRAME;
    }
    if (_styleFlags & GERIUM_APPLICATION_STYLE_MINIMIZABLE_BIT) {
        result |= WS_MINIMIZEBOX;
    }
    if (_styleFlags & GERIUM_APPLICATION_STYLE_MAXIMIZABLE_BIT) {
        result |= WS_MAXIMIZEBOX;
    }
    return result;
}

std::pair<gerium_uint16_t, gerium_uint16_t> Win32Application::clientSize() const noexcept {
    RECT rect;
    GetClientRect(_hWnd, &rect);

    auto currentWidth  = gerium_uint16_t(rect.right - rect.left);
    auto currentHeight = gerium_uint16_t(rect.bottom - rect.top);
    return { currentWidth, currentHeight };
}

std::pair<gerium_uint16_t, gerium_uint16_t> Win32Application::clientSizeToWindowSize(
    gerium_uint16_t width, gerium_uint16_t height) const noexcept {
    const auto style = getStyle();

    RECT rect;
    rect.left   = 0;
    rect.right  = (LONG) width;
    rect.top    = 0;
    rect.bottom = (LONG) height;
    AdjustWindowRect(&rect, style, FALSE);

    auto winWidth  = gerium_uint16_t(rect.right - rect.left);
    auto winHeight = gerium_uint16_t(rect.bottom - rect.top);
    return { winWidth, winHeight };
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

                info.id         = i;
                info.mode_count = 0;

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
        DispatchMessageW(pMsg);
        if (GetActiveWindow()) {
            return true;
        }
    }
    return false;
}

std::wstring Win32Application::wideString(gerium_utf8_t utf8) {
    auto byteCount = (int) std::strlen(utf8);
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
    auto application = (Win32Application*) GetWindowLongPtrW(hWnd, GWLP_USERDATA);
    return application ? application->wndProc(message, wParam, lParam) : DefWindowProcW(hWnd, message, wParam, lParam);
}

} // namespace gerium::windows

gerium_result_t gerium_application_create(gerium_utf8_t title,
                                          gerium_uint32_t width,
                                          gerium_uint32_t height,
                                          gerium_application_t* application) {
    return gerium_windows_application_create(title, width, height, nullptr, application);
}

gerium_result_t gerium_windows_application_create(gerium_utf8_t title,
                                                  gerium_uint32_t width,
                                                  gerium_uint32_t height,
                                                  HINSTANCE instance,
                                                  gerium_application_t* application) {
    using namespace gerium;
    using namespace gerium::windows;
    return Object::create<Win32Application>(*application, title, width, height, instance);
}
