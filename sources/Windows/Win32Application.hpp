#ifndef GERIUM_WINDOWS_WIN32_APPLICATION_HPP
#define GERIUM_WINDOWS_WIN32_APPLICATION_HPP

#include "../Application.hpp"

namespace gerium::windows {

class Win32Application final : public Application {
public:
    Win32Application(gerium_utf8_t title,
                     gerium_uint32_t width,
                     gerium_uint32_t height,
                     gerium_application_mode_flags_t mode,
                     HINSTANCE instance);

private:
    gerium_runtime_platform_t onGetPlatform() const noexcept override;
    void onRun() override;
    void onExit() noexcept override;

    LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam);

    static bool waitInBackground(LPMSG pMsg);
    static std::wstring wideString(gerium_utf8_t utf8);
    static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    static constexpr wchar_t _kClassName[] = L"Gerium";

    HINSTANCE _hInstance;
    HWND _hWnd;
    bool _running;
};

} // namespace gerium::windows

#endif
