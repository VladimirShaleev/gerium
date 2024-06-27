#ifndef GERIUM_WINDOWS_WIN32_APPLICATION_HPP
#define GERIUM_WINDOWS_WIN32_APPLICATION_HPP

#include "../Application.hpp"

namespace gerium::windows {

class Win32Application final : public Application {
public:
    Win32Application(gerium_utf8_t title,
                     gerium_uint32_t width,
                     gerium_uint32_t height,
                     // gerium_application_mode_flags_t mode,
                     HINSTANCE instance);

private:
    gerium_runtime_platform_t onGetPlatform() const noexcept override;

    void onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const override;

    bool onGetFullscreen() const noexcept override;
    void onSetFullscreen(bool fullscreen) noexcept override;

    void onRun() override;
    void onExit() noexcept override;

    LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam);

    void saveWindowPlacement();
    void restoreWindowPlacement();
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
    LONG _style;
    LONG _styleEx;
    mutable std::map<std::wstring, std::string> _monitors;
    mutable std::vector<gerium_display_mode_t> _modes;
    mutable std::vector<std::string> _displayNames;
};

} // namespace gerium::windows

#endif
