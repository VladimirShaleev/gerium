#ifndef GERIUM_LINUX_LINUX_APPLICATION_HPP
#define GERIUM_LINUX_LINUX_APPLICATION_HPP

#include "../Application.hpp"

#include <X11/Xatom.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xinerama.h>

namespace gerium::linux {

class LinuxApplication final : public Application {
public:
    LinuxApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height);
    ~LinuxApplication() override;

    xcb_connection_t* connection() const noexcept;
    uint32_t window() const noexcept;

private:
    using Object::error;

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

    void onRun() override;
    void onExit() noexcept override;

    bool onIsRunning() const noexcept override;

    void onInitImGui() override;
    void onShutdownImGui() override;
    void onNewFrameImGui() override;

    void decorate(gerium_application_style_flags_t styles);

    xcb_atom_t getAtom(std::string_view name, bool onlyIfExists = false) const;
    std::vector<xcb_atom_t> getValueAtoms(xcb_atom_t atom) const;

    bool isVisible() const;

    template <typename T>
    std::vector<T> getProperties(Atom property, Atom type) const {
        Atom actualType;
        int actualFormat;
        unsigned long itemCount;
        unsigned long bytesAfter;
        unsigned char* values;

        XGetWindowProperty(_display,
                           _window,
                           property,
                           0,
                           LONG_MAX,
                           False,
                           type,
                           &actualType,
                           &actualFormat,
                           &itemCount,
                           &bytesAfter,
                           &values);

        std::vector<T> result;
        if (values) {
            result.resize(itemCount);
            memcpy(result.data(), values, itemCount * sizeof(T));
            XFree(values);
        }
        return result;
    }

    void showWindow();
    void focusWindow();

    void sendWMEvent(Atom type, long a1, long a2, long a3, long a4, long a5);

    void handleEvent(const XEvent& event);
    void pollEvents();
    void waitEvents();
    void waitVisible();
    void waitActive();

    void error(gerium_result_t error, const std::string_view message, bool throwError = false);

    void acquirereErrorHandler();
    void releaseErrorHandler();

    static int errorHandler(Display* display, XErrorEvent* event);

    struct ErrorGuard {
        ErrorGuard(const ErrorGuard&) = delete;
        ErrorGuard(ErrorGuard&&)      = delete;

        ErrorGuard& operator=(const ErrorGuard&) = delete;
        ErrorGuard& operator=(ErrorGuard&&)      = delete;

        explicit ErrorGuard(LinuxApplication* app) noexcept : _app(app) {
            app->acquirereErrorHandler();
        }

        ~ErrorGuard() {
            _app->releaseErrorHandler();
        }

        LinuxApplication* _app;
    };

    static int _errorCode;
    static Display* _display;
    XErrorHandler _errorHandler{};
    int _screen{};
    Window _root{};
    Colormap _colormap{};
    XContext _context{};
    Window _window{};
    gerium_uint16_t _width{};
    gerium_uint16_t _height{};
    std::string _title{};
    bool _running{};
    bool _active{};
    bool _resizing{};
    std::chrono::steady_clock::time_point _lastResizeTime{};
    ObjectPtr<Logger> _logger{};

    Atom UTF8_STRING{};
    Atom WM_PROTOCOLS{};
    Atom WM_STATE{};
    Atom WM_DELETE_WINDOW{};
    Atom NET_ACTIVE_WINDOW{};
    Atom NET_WM_NAME{};
    Atom NET_WM_ICON_NAME{};
    Atom NET_WM_ICON{};
    Atom NET_WM_PID{};
    Atom NET_WM_PING{};
    Atom NET_WM_WINDOW_TYPE{};
    Atom NET_WM_WINDOW_TYPE_NORMAL{};
    Atom NET_WM_STATE{};
    Atom NET_WM_STATE_ABOVE{};
    Atom NET_WM_STATE_FULLSCREEN{};
    Atom NET_WM_STATE_MAXIMIZED_VERT{};
    Atom NET_WM_STATE_MAXIMIZED_HORZ{};
    Atom NET_WM_FULLSCREEN_MONITORS{};
    Atom MOTIF_WM_HINTS{};
};

} // namespace gerium::linux

#endif
