#ifndef GERIUM_WINDOWS_WIN32_APPLICATION_HPP
#define GERIUM_WINDOWS_WIN32_APPLICATION_HPP

#include "../Application.hpp"
#include "Win32ScanCodes.hpp"

namespace gerium::windows {

class Win32Application final : public Application {
public:
    Win32Application(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height, HINSTANCE instance);
    ~Win32Application() override;

    HINSTANCE hInstance() const noexcept;
    HWND hWnd() const noexcept;

private:
    static constexpr gerium_float32_t kInchesPerMm = 1.0f / 25.4f;
    static constexpr gerium_float32_t kInchesPerPt = 1.0f / 72.0f;

    enum class MouseEventSource {
        Mouse,
        Touch,
        Pen
    };

    gerium_runtime_platform_t onGetPlatform() const noexcept override;

    void onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const override;

    bool onIsFullscreen() const noexcept override;
    void onFullscreen(bool fullscreen, gerium_uint32_t displayId, const gerium_display_mode_t* mode) override;

    gerium_application_style_flags_t onGetStyle() const noexcept override;
    void onSetStyle(gerium_application_style_flags_t style) noexcept override;

    void onGetMinSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept override;
    void onGetMaxSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept override;
    void onGetSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept override;
    void onSetMinSize(gerium_uint16_t width, gerium_uint16_t height) noexcept override;
    void onSetMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept override;
    void onSetSize(gerium_uint16_t width, gerium_uint16_t height) noexcept override;

    gerium_utf8_t onGetTitle() const noexcept override;
    void onSetTitle(gerium_utf8_t title) noexcept override;

    void onShowCursor(bool show) noexcept override;

    gerium_float32_t onGetDensity() const noexcept override;
    gerium_float32_t onGetDimension(gerium_dimension_unit_t unit, gerium_float32_t value) const noexcept override;

    void onRun() override;
    void onExit() noexcept override;

    void onShowMessage(gerium_utf8_t title, gerium_utf8_t message) noexcept override;

    bool onIsRunning() const noexcept override;

    void onInitImGui() override;
    void onShutdownImGui() override;
    void onNewFrameImGui() override;

    LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    LRESULT inputProc(UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    void inputThread() noexcept;
    void pollInput(LARGE_INTEGER frequency, HKL lang) noexcept;

    void saveWindowPlacement();
    void restoreWindowPlacement();
    LONG getStyle() const noexcept;
    std::pair<gerium_uint16_t, gerium_uint16_t> clientSize() const noexcept;
    std::pair<gerium_uint16_t, gerium_uint16_t> clientSizeToWindowSize(gerium_uint16_t width,
                                                                       gerium_uint16_t height) const noexcept;
    void enumDisplays(gerium_uint32_t displayCount,
                      gerium_uint32_t& displayIndex,
                      bool primary,
                      gerium_display_info_t* displays) const;
    void createInputThread();
    void closeInputThread();

    void captureCursor(bool capture) noexcept;
    gerium_float32_t dpi() const noexcept;
    gerium_float32_t scaledDensity() const noexcept;

    static bool waitInBackground(LPMSG pMsg);
    static std::wstring wideString(gerium_utf8_t utf8);
    static std::string utf8String(const std::wstring& wstr);
    static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK inputProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static DWORD WINAPI inputThread(LPVOID lpThreadParameter);
    static ScanCode getScanCode(const RAWKEYBOARD& keyboard, bool& keyUp) noexcept;
    static bool isPenEvent(const RAWMOUSE& mouse) noexcept;
    static MouseEventSource getMouseEventSource(const RAWMOUSE& mouse) noexcept;

    static constexpr wchar_t _kClassName[] = L"Gerium";
    static constexpr wchar_t _kInputName[] = L"Input";

    HINSTANCE _hInstance;
    HWND _hWnd;
    HMONITOR _hMonitor;
    mutable std::string _title;
    std::atomic_bool _running;
    bool _resizing;
    bool _visibility;
    mutable bool _needUpdateDPI;
    mutable bool _needUpdateScaledDensity;
    mutable gerium_float32_t _dpi;
    mutable gerium_float32_t _scaledDensity;
    WINDOWPLACEMENT _windowPlacement;
    LONG _styleEx;
    gerium_application_style_flags_t _styleFlags;
    gerium_uint16_t _minWidth;
    gerium_uint16_t _minHeight;
    gerium_uint16_t _maxWidth;
    gerium_uint16_t _maxHeight;
    gerium_uint16_t _newWidth;
    gerium_uint16_t _newHeight;
    mutable std::map<std::wstring, std::string> _monitors;
    mutable std::vector<gerium_display_mode_t> _modes;
    mutable std::vector<std::string> _displayNames;
    marl::Scheduler* _scheduler;
    HANDLE _inputThread;
    HANDLE _readyInputEvent;
    HANDLE _shutdownInputEvent;
    RAWINPUT _rawInput[16];
    std::atomic_bool _capslock;
    std::atomic_bool _numlock;
    std::atomic_bool _scrolllock;
    POINT _lastMousePos;
    gerium_uint64_t _lastInputTimestamp;
};

} // namespace gerium::windows

#endif
