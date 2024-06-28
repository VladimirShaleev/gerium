#ifndef GERIUM_WINDOWS_WIN32_APPLICATION_HPP
#define GERIUM_WINDOWS_WIN32_APPLICATION_HPP

#include "../Application.hpp"

namespace gerium::windows {

class Win32Application final : public Application {
public:
    Win32Application(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height, HINSTANCE instance);

private:
    gerium_runtime_platform_t onGetPlatform() const noexcept override;

    void onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const override;

    bool onIsFullscreen() const noexcept override;
    void onFullscreen(bool fullscreen, const gerium_display_mode_t* mode) override;

    gerium_application_style_flags_t onGetStyle() const noexcept override;
    void onSetStyle(gerium_application_style_flags_t style) noexcept override;

    void onSetMinSize(gerium_uint16_t width, gerium_uint16_t height) noexcept override;
    void onSetMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept override;
    void onSetSize(gerium_uint16_t width, gerium_uint16_t height) noexcept override;

    void onRun() override;
    void onExit() noexcept override;

    LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam);

    void saveWindowPlacement();
    void restoreWindowPlacement();
    bool changeState(gerium_application_state_t newState) noexcept;
    LONG getStyle() const noexcept;
    std::pair<gerium_uint16_t, gerium_uint16_t> clientSize() const noexcept;
    std::pair<gerium_uint16_t, gerium_uint16_t> clientSizeToWindowSize(gerium_uint16_t width,
                                                                       gerium_uint16_t height) const noexcept;
    void enumDisplays(gerium_uint32_t displayCount,
                      gerium_uint32_t& displayIndex,
                      bool primary,
                      gerium_display_info_t* displays) const;

    static bool waitInBackground(LPMSG pMsg);
    static std::wstring wideString(gerium_utf8_t utf8);
    static std::string utf8String(const std::wstring& wstr);
    static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    static constexpr wchar_t _kClassName[] = L"Gerium";

    HINSTANCE _hInstance;
    HWND _hWnd;
    bool _running;
    bool _resizing;
    bool _visibility;
    WINDOWPLACEMENT _windowPlacement;
    LONG _styleEx;
    gerium_application_style_flags_t _styleFlags;
    gerium_application_state_t _prevState;
    gerium_uint16_t _minWidth;
    gerium_uint16_t _minHeight;
    gerium_uint16_t _maxWidth;
    gerium_uint16_t _maxHeight;
    mutable std::map<std::wstring, std::string> _monitors;
    mutable std::vector<gerium_display_mode_t> _modes;
    mutable std::vector<std::string> _displayNames;
};

} // namespace gerium::windows

#endif
