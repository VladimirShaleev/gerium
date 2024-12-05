#ifndef GERIUM_LINUX_LINUX_APPLICATION_HPP
#define GERIUM_LINUX_LINUX_APPLICATION_HPP

#include "../Application.hpp"

#include <X11/Xatom.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/Xrandr.h>

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

    void restoreMode();
    void setMode();
    void sendWMEvent(Atom type, long a1, long a2, long a3, long a4, long a5);
    void setNormalHints(gerium_uint16_t minWidth,
                        gerium_uint16_t minHeight,
                        gerium_uint16_t maxWidth,
                        gerium_uint16_t maxHeight);

    void handleEvent(XEvent& event);
    void pollEvents();
    void waitEvents();
    void waitVisible();
    void waitActive();

    void error(gerium_result_t error, const std::string_view message, bool throwError = false);

    void acquirereErrorHandler();
    void releaseErrorHandler();

    static int errorHandler(Display* display, XErrorEvent* event);
    static void destroyIm(XIM im, XPointer clientData, XPointer callData);
    static void destroyIc(XIM im, XPointer clientData, XPointer callData);

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

    struct XInput2Table {
        typedef decltype(&::XIQueryVersion) PFN_XIQueryVersion;
        typedef decltype(&::XISelectEvents) PFN_XISelectEvents;

        XInput2Table();
        ~XInput2Table();

        PFN_XIQueryVersion XIQueryVersion;
        PFN_XISelectEvents XISelectEvents;

        void* dll{};
        int extension{};
    };

    struct XineramaTable {
        typedef decltype(&::XineramaIsActive) PFN_XineramaIsActive;
        typedef decltype(&::XineramaQueryExtension) PFN_XineramaQueryExtension;
        typedef decltype(&::XineramaQueryScreens) PFN_XineramaQueryScreens;

        XineramaTable();
        ~XineramaTable();

        void setup();

        PFN_XineramaIsActive XineramaIsActive;
        PFN_XineramaQueryExtension XineramaQueryExtension;
        PFN_XineramaQueryScreens XineramaQueryScreens;

        void* dll{};
        int major{};
        int minor{};
        bool available{};
    };

    struct XRandrTable {
        typedef decltype(&::XRRAllocGamma) PFN_XRRAllocGamma;
        typedef decltype(&::XRRFreeCrtcInfo) PFN_XRRFreeCrtcInfo;
        typedef decltype(&::XRRFreeGamma) PFN_XRRFreeGamma;
        typedef decltype(&::XRRFreeOutputInfo) PFN_XRRFreeOutputInfo;
        typedef decltype(&::XRRFreeScreenResources) PFN_XRRFreeScreenResources;
        typedef decltype(&::XRRGetCrtcGamma) PFN_XRRGetCrtcGamma;
        typedef decltype(&::XRRGetCrtcGammaSize) PFN_XRRGetCrtcGammaSize;
        typedef decltype(&::XRRGetCrtcInfo) PFN_XRRGetCrtcInfo;
        typedef decltype(&::XRRGetOutputInfo) PFN_XRRGetOutputInfo;
        typedef decltype(&::XRRGetOutputPrimary) PFN_XRRGetOutputPrimary;
        typedef decltype(&::XRRGetScreenResourcesCurrent) PFN_XRRGetScreenResourcesCurrent;
        typedef decltype(&::XRRQueryExtension) PFN_XRRQueryExtension;
        typedef decltype(&::XRRQueryVersion) PFN_XRRQueryVersion;
        typedef decltype(&::XRRSelectInput) PFN_XRRSelectInput;
        typedef decltype(&::XRRSetCrtcConfig) PFN_XRRSetCrtcConfig;
        typedef decltype(&::XRRSetCrtcGamma) PFN_XRRSetCrtcGamma;
        typedef decltype(&::XRRUpdateConfiguration) PFN_XRRUpdateConfiguration;

        XRandrTable();
        ~XRandrTable();

        void setup(Window root);

        PFN_XRRAllocGamma XRRAllocGamma;
        PFN_XRRFreeCrtcInfo XRRFreeCrtcInfo;
        PFN_XRRFreeGamma XRRFreeGamma;
        PFN_XRRFreeOutputInfo XRRFreeOutputInfo;
        PFN_XRRFreeScreenResources XRRFreeScreenResources;
        PFN_XRRGetCrtcGamma XRRGetCrtcGamma;
        PFN_XRRGetCrtcGammaSize XRRGetCrtcGammaSize;
        PFN_XRRGetCrtcInfo XRRGetCrtcInfo;
        PFN_XRRGetOutputInfo XRRGetOutputInfo;
        PFN_XRRGetOutputPrimary XRRGetOutputPrimary;
        PFN_XRRGetScreenResourcesCurrent XRRGetScreenResourcesCurrent;
        PFN_XRRQueryExtension XRRQueryExtension;
        PFN_XRRQueryVersion XRRQueryVersion;
        PFN_XRRSelectInput XRRSelectInput;
        PFN_XRRSetCrtcConfig XRRSetCrtcConfig;
        PFN_XRRSetCrtcGamma XRRSetCrtcGamma;
        PFN_XRRUpdateConfiguration XRRUpdateConfiguration;

        void* dll{};
        int eventBase{};
        int errorBase{};
        int major{};
        int minor{};
        bool available{};
        bool gammaBroken{};
    };

    struct GeriumDisplay {
        int index;
        RRCrtc crtc;
        RROutput output;
        gerium_uint32_t id;
        std::string name;
        std::vector<gerium_display_mode_t> modes;
        std::vector<RRMode> modeIds;
    };

    static constexpr auto kNoValue = std::numeric_limits<gerium_uint16_t>::max();

    static int _errorCode;
    static Display* _display;
    XInput2Table _xinput{};
    XineramaTable _xinerama{};
    XRandrTable _randr{};
    XErrorHandler _errorHandler{};
    int _screen{};
    Window _root{};
    Colormap _colormap{};
    XContext _context{};
    Window _window{};
    XIM _im{};
    XIC _ic{};
    gerium_application_style_flags_t _styles{};
    gerium_uint16_t _width{};
    gerium_uint16_t _height{};
    gerium_uint16_t _minWidth{ kNoValue };
    gerium_uint16_t _minHeight{ kNoValue };
    gerium_uint16_t _maxWidth{ kNoValue };
    gerium_uint16_t _maxHeight{ kNoValue };
    gerium_uint16_t _originWidth{};
    gerium_uint16_t _originHeight{};
    RRMode _originMode{};
    gerium_uint32_t _displayId{};
    std::optional<gerium_display_mode_t> _displayMode{};
    std::string _title{};
    bool _running{};
    bool _active{};
    bool _resizing{};
    bool _fullscreen{};
    gerium_uint64_t _lastInputTimestamp{};
    std::chrono::steady_clock::time_point _lastResizeTime{};
    mutable std::vector<GeriumDisplay> _displays{};
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
