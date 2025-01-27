#include "LinuxApplication.hpp"
#include "LinuxScanCodes.hpp"

namespace gerium::linux {

LinuxApplication::LinuxApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) :
    _logger(Logger::create("gerium:application:x11")) {
    _display = _x11.XOpenDisplay(nullptr);

    if (!_display) {
        error(GERIUM_RESULT_ERROR_NO_DISPLAY);
    }

    _styles = GERIUM_APPLICATION_STYLE_RESIZABLE_BIT | GERIUM_APPLICATION_STYLE_MINIMIZABLE_BIT |
              GERIUM_APPLICATION_STYLE_MAXIMIZABLE_BIT;
    _width  = width;
    _height = height;

    auto visual = DefaultVisual(_display, _screen);
    auto depth  = DefaultDepth(_display, _screen);

    _screen   = DefaultScreen(_display);
    _root     = RootWindow(_display, _screen);
    _colormap = _x11.XCreateColormap(_display, _root, visual, AllocNone);
    _context  = (XContext) _x11.XrmUniqueQuark();

    _xinput.setup(_x11);
    _xinerama.setup();
    _randr.setup(_root);

    UTF8_STRING                 = _x11.XInternAtom(_display, "UTF8_STRING", False);
    WM_PROTOCOLS                = _x11.XInternAtom(_display, "WM_PROTOCOLS", False);
    WM_STATE                    = _x11.XInternAtom(_display, "WM_STATE", False);
    WM_DELETE_WINDOW            = _x11.XInternAtom(_display, "WM_DELETE_WINDOW", False);
    NET_ACTIVE_WINDOW           = _x11.XInternAtom(_display, "_NET_ACTIVE_WINDOW", False);
    NET_WM_NAME                 = _x11.XInternAtom(_display, "_NET_WM_NAME", False);
    NET_WM_ICON_NAME            = _x11.XInternAtom(_display, "_NET_WM_ICON_NAME", False);
    NET_WM_ICON                 = _x11.XInternAtom(_display, "_NET_WM_ICON", False);
    NET_WM_PID                  = _x11.XInternAtom(_display, "_NET_WM_PID", False);
    NET_WM_PING                 = _x11.XInternAtom(_display, "_NET_WM_PING", False);
    NET_WM_WINDOW_TYPE          = _x11.XInternAtom(_display, "_NET_WM_WINDOW_TYPE", False);
    NET_WM_WINDOW_TYPE_NORMAL   = _x11.XInternAtom(_display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    NET_WM_STATE                = _x11.XInternAtom(_display, "_NET_WM_STATE", False);
    NET_WM_STATE_ABOVE          = _x11.XInternAtom(_display, "_NET_WM_STATE_ABOVE", False);
    NET_WM_STATE_FULLSCREEN     = _x11.XInternAtom(_display, "_NET_WM_STATE_FULLSCREEN", False);
    NET_WM_STATE_MAXIMIZED_VERT = _x11.XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    NET_WM_STATE_MAXIMIZED_HORZ = _x11.XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    NET_WM_FULLSCREEN_MONITORS  = _x11.XInternAtom(_display, "_NET_WM_FULLSCREEN_MONITORS ", False);
    MOTIF_WM_HINTS              = _x11.XInternAtom(_display, "_MOTIF_WM_HINTS", False);

    XSetWindowAttributes wa{};
    wa.colormap   = _colormap;
    wa.event_mask = StructureNotifyMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | ExposureMask |
                    FocusChangeMask | VisibilityChangeMask | EnterWindowMask | LeaveWindowMask | PropertyChangeMask;

    {
        ErrorGuard error(this);
        _window = _x11.XCreateWindow(_display,
                                     _root,
                                     0,
                                     0,
                                     width,
                                     height,
                                     0,
                                     depth,
                                     InputOutput,
                                     visual,
                                     CWBorderPixel | CWColormap | CWEventMask,
                                     &wa);
    }
    error(GERIUM_RESULT_ERROR_APPLICATION_CREATE, "create X11 window failed", true);

    _x11.XSaveContext(_display, _window, _context, (XPointer) this);

    Atom protocols[] = { WM_DELETE_WINDOW, NET_WM_PING };
    _x11.XSetWMProtocols(_display, _window, protocols, std::size(protocols));

    const long pid = getpid();
    _x11.XChangeProperty(_display, _window, NET_WM_PID, XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &pid, 1);

    if (NET_WM_WINDOW_TYPE && NET_WM_WINDOW_TYPE_NORMAL) {
        auto type = NET_WM_WINDOW_TYPE_NORMAL;
        _x11.XChangeProperty(
            _display, _window, NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (unsigned char*) &type, 1);
    }

    auto wmHints = _x11.XAllocWMHints();
    if (!wmHints) {
        error(GERIUM_RESULT_ERROR_OUT_OF_MEMORY);
    }
    wmHints->flags         = StateHint;
    wmHints->initial_state = NormalState;
    _x11.XSetWMHints(_display, _window, wmHints);
    _x11.XFree(wmHints);

    auto sizeHints = _x11.XAllocSizeHints();
    if (!sizeHints) {
        error(GERIUM_RESULT_ERROR_OUT_OF_MEMORY);
    }
    sizeHints->flags |= PWinGravity;
    sizeHints->win_gravity = StaticGravity;
    _x11.XSetWMNormalHints(_display, _window, sizeHints);
    _x11.XFree(sizeHints);

    auto classHint = _x11.XAllocClassHint();
    if (!classHint) {
        error(GERIUM_RESULT_ERROR_OUT_OF_MEMORY);
    }
    auto name            = strlen(title) ? (char*) title : (char*) "gerium";
    classHint->res_name  = name;
    classHint->res_class = name;
    _x11.XSetClassHint(_display, _window, classHint);
    _x11.XFree(classHint);

    onSetTitle(title);

    _im = _x11.XOpenIM(_display, nullptr, nullptr, nullptr);
    if (_im) {
        bool found        = false;
        XIMStyles* styles = NULL;

        if (!_x11.XGetIMValues(_im, XNQueryInputStyle, &styles, NULL)) {
            for (unsigned int i = 0; i < styles->count_styles; i++) {
                if (styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing)) {
                    found = true;
                    break;
                }
            }
            _x11.XFree(styles);
        }

        if (!found) {
            _x11.XCloseIM(_im);
            _im = nullptr;
        }
    }

    if (_im) {
        XIMCallback callbackIm;
        callbackIm.callback    = destroyIm;
        callbackIm.client_data = (XPointer) this;
        _x11.XSetIMValues(_im, XNDestroyCallback, &callbackIm, nullptr);

        XIMCallback callbackIc;
        callbackIc.callback    = destroyIc;
        callbackIc.client_data = (XPointer) this;

        _ic = _x11.XCreateIC(_im,
                             XNInputStyle,
                             XIMPreeditNothing | XIMStatusNothing,
                             XNClientWindow,
                             _window,
                             XNFocusWindow,
                             _window,
                             XNDestroyCallback,
                             &callbackIc,
                             nullptr);

        if (_ic) {
            XWindowAttributes attribs;
            _x11.XGetWindowAttributes(_display, _window, &attribs);

            unsigned long filter = 0;
            if (_x11.XGetICValues(_ic, XNFilterEvents, &filter, NULL) == NULL) {
                _x11.XSelectInput(_display, _window, attribs.your_event_mask | filter);
            }
        }
    }

    unsigned char masks[XIMaskLen(XI_LASTEVENT)]{};
    XIEventMask xiMask{};
    xiMask.deviceid = XIAllMasterDevices;
    xiMask.mask_len = sizeof(masks);
    xiMask.mask     = masks;
    XISetMask(xiMask.mask, XI_KeyPress);
    XISetMask(xiMask.mask, XI_KeyRelease);
    XISetMask(xiMask.mask, XI_ButtonPress);
    XISetMask(xiMask.mask, XI_ButtonRelease);
    XISetMask(xiMask.mask, XI_Motion);

    _xinput.XISelectEvents(_display, _window, &xiMask, 1);
    _x11.XSync(_display, False);

    calcDensity();
}

LinuxApplication::~LinuxApplication() {
    if (_ic) {
        _x11.XDestroyIC(_ic);
    }
    if (_im) {
        _x11.XCloseIM(_im);
    }

    if (_display) {
        if (_window) {
            _x11.XDeleteContext(_display, _window, _context);
            _x11.XUnmapWindow(_display, _window);
            _x11.XDestroyWindow(_display, _window);
        }
        if (_colormap) {
            _x11.XFreeColormap(_display, _colormap);
        }
        _x11.XFlush(_display);
        _x11.XCloseDisplay(_display);
    }
}

xcb_connection_t* LinuxApplication::connection() const noexcept {
    return _x11xcb.XGetXCBConnection(_display);
}

uint32_t LinuxApplication::window() const noexcept {
    return _window;
}

gerium_runtime_platform_t LinuxApplication::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_LINUX;
}

void LinuxApplication::onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const {
    if (!_randr.available) {
        displayCount = 0;
        return;
    }

    _displays.clear();

    int disconnectedCount, screenCount = 0;
    XineramaScreenInfo* screens = nullptr;

    auto sr = _randr.XRRGetScreenResourcesCurrent(_display, _root);

    if (_xinerama.available) {
        screens = _xinerama.XineramaQueryScreens(_display, &screenCount);
    }

    for (int i = 0; i < sr->noutput; i++) {
        _displays.push_back({});

        auto oi = _randr.XRRGetOutputInfo(_display, sr, sr->outputs[i]);
        if (oi->connection != RR_Connected || oi->crtc == None) {
            _randr.XRRFreeOutputInfo(oi);
            continue;
        }

        auto& display = _displays.back();

        auto ci = _randr.XRRGetCrtcInfo(_display, sr, oi->crtc);
        for (int j = 0; j < screenCount; j++) {
            if (screens[j].x_org == ci->x && screens[j].y_org == ci->y && screens[j].width == ci->width &&
                screens[j].height == ci->height) {
                display.index = j;
                break;
            }
        }

        display.crtc   = oi->crtc;
        display.output = sr->outputs[i];
        display.id     = i;
        display.name   = oi->name;

        display.modes.resize(oi->nmode);
        display.modeIds.resize(oi->nmode);

        for (int m = 0; m < oi->nmode; ++m) {
            XRRModeInfo* mode;
            for (int mi = 0; mi < sr->nmode; ++mi) {
                if (sr->modes[mi].id == oi->modes[m]) {
                    mode = &sr->modes[mi];
                    break;
                }
            }

            auto tv = mode->hTotal * mode->vTotal;

            auto refreshRate = tv ? gerium_uint16_t(std::round((double) mode->dotClock / tv)) : 0;

            display.modeIds[m] = mode->id;
            if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270) {
                display.modes[m].width  = mode->height;
                display.modes[m].height = mode->width;

            } else {
                display.modes[m].width  = mode->width;
                display.modes[m].height = mode->height;
            }
            display.modes[m].refresh_rate = refreshRate;
        }

        _randr.XRRFreeOutputInfo(oi);
        _randr.XRRFreeCrtcInfo(ci);
    }

    _randr.XRRFreeScreenResources(sr);

    if (screens) {
        _x11.XFree(screens);
    }

    if (displays) {
        gerium_uint32_t count = 0;
        for (; count < displayCount && count < _displays.size(); ++count) {
            displays[count].id          = _displays[count].id;
            displays[count].name        = _displays[count].name.c_str();
            displays[count].gpu_name    = "Unknown";
            displays[count].device_name = "Unknown";
            displays[count].mode_count  = (gerium_uint32_t) _displays[count].modes.size();
            displays[count].modes       = _displays[count].modes.data();
        }
        displayCount = count;
    } else {
        displayCount = (gerium_uint32_t) _displays.size();
    }
}

bool LinuxApplication::onIsFullscreen() const noexcept {
    return _fullscreen;
}

void LinuxApplication::onFullscreen(bool fullscreen, gerium_uint32_t displayId, const gerium_display_mode_t* mode) {
    if (!isVisible()) {
        _x11.XMapRaised(_display, _window);
        waitVisible();
    }

    if (NET_WM_STATE && NET_WM_STATE_FULLSCREEN) {
        _displayId   = displayId;
        _displayMode = mode ? std::make_optional(*mode) : std::nullopt;

        if (_xinerama.available && NET_WM_FULLSCREEN_MONITORS) {
            if (fullscreen) {
                auto it = std::find_if(_displays.cbegin(), _displays.cend(), [displayId](const auto& display) {
                    return display.id == displayId;
                });
                if (it != _displays.cend()) {
                    auto index = it->index;
                    sendWMEvent(NET_WM_FULLSCREEN_MONITORS, index, index, index, index, 0);
                }
            } else {
                _x11.XDeleteProperty(_display, _window, NET_WM_FULLSCREEN_MONITORS);
            }
        }

        if (!_fullscreen && fullscreen) {
            _originWidth  = _width;
            _originHeight = _height;
        } else {
            _width  = _originWidth;
            _height = _originHeight;
        }
        _fullscreen = fullscreen;
        onSetStyle(_styles);
        sendWMEvent(NET_WM_STATE, fullscreen ? 1 : 0, NET_WM_STATE_FULLSCREEN, 0, 1, 0);

        _x11.XFlush(_display);
        // TODO:
        //   It is not yet clear how to correctly switch from full-screen mode to
        //   windowed mode with the restoration of window decoders without this
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        onSetStyle(_styles);
        _x11.XFlush(_display);
    }
}

gerium_application_style_flags_t LinuxApplication::onGetStyle() const noexcept {
    return _styles;
}

void LinuxApplication::onSetStyle(gerium_application_style_flags_t style) noexcept {
    constexpr uint32_t MWM_HINTS_FUNCTIONS   = 1L << 0;
    constexpr uint32_t MWM_HINTS_DECORATIONS = 1L << 1;
    constexpr uint32_t MWM_HINTS_INPUT_MODE  = 1L << 2;
    constexpr uint32_t MWM_HINTS_STATUS      = 1L << 3;

    constexpr uint32_t MWM_FUNC_ALL      = 1L << 0;
    constexpr uint32_t MWM_FUNC_RESIZE   = 1L << 1;
    constexpr uint32_t MWM_FUNC_MOVE     = 1L << 2;
    constexpr uint32_t MWM_FUNC_MINIMIZE = 1L << 3;
    constexpr uint32_t MWM_FUNC_MAXIMIZE = 1L << 4;
    constexpr uint32_t MWM_FUNC_CLOSE    = 1L << 5;

    constexpr uint32_t MWM_DECOR_ALL      = 1L << 0;
    constexpr uint32_t MWM_DECOR_BORDER   = 1L << 1;
    constexpr uint32_t MWM_DECOR_RESIZE   = 1L << 2;
    constexpr uint32_t MWM_DECOR_TITLE    = 1L << 3;
    constexpr uint32_t MWM_DECOR_MENU     = 1L << 4;
    constexpr uint32_t MWM_DECOR_MINIMIZE = 1L << 5;
    constexpr uint32_t MWM_DECOR_MAXIMIZE = 1L << 6;

    struct {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long input_mode;
        unsigned long status;
    } hints = {};

    hints.flags = MWM_HINTS_DECORATIONS | MWM_HINTS_FUNCTIONS;

    if (!_fullscreen) {
        hints.decorations |= MWM_DECOR_BORDER | MWM_DECOR_TITLE;
        hints.functions |= MWM_FUNC_MOVE | MWM_FUNC_CLOSE;

        if (style & GERIUM_APPLICATION_STYLE_RESIZABLE_BIT) {
            hints.decorations |= MWM_DECOR_RESIZE;
            hints.functions |= MWM_FUNC_RESIZE;
        }
        if (style & GERIUM_APPLICATION_STYLE_MINIMIZABLE_BIT) {
            hints.decorations |= MWM_DECOR_MINIMIZE;
            hints.functions |= MWM_FUNC_MINIMIZE;
        }
        if (style & GERIUM_APPLICATION_STYLE_MAXIMIZABLE_BIT) {
            hints.decorations |= MWM_DECOR_MAXIMIZE;
            hints.functions |= MWM_FUNC_MAXIMIZE;
        }

        _x11.XChangeProperty(_display,
                             _window,
                             MOTIF_WM_HINTS,
                             MOTIF_WM_HINTS,
                             32,
                             PropModeReplace,
                             (unsigned char*) &hints,
                             sizeof(hints) / sizeof(long));
    } else {
        _x11.XChangeProperty(_display,
                             _window,
                             MOTIF_WM_HINTS,
                             MOTIF_WM_HINTS,
                             32,
                             PropModeReplace,
                             (unsigned char*) &hints,
                             sizeof(hints) / sizeof(long));
    }
    onSetSize(_width, _height);

    _styles = style;
}

void LinuxApplication::onGetMinSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    if (width) {
        *width = _minWidth;
    }
    if (height) {
        *height = _minHeight;
    }
}

void LinuxApplication::onGetMaxSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    if (width) {
        *width = _maxWidth;
    }
    if (height) {
        *height = _maxHeight;
    }
}

void LinuxApplication::onGetSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    if (width) {
        *width = _width;
    }
    if (height) {
        *height = _height;
    }
}

void LinuxApplication::onSetMinSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    setNormalHints(width, height, _maxWidth, _maxHeight);
}

void LinuxApplication::onSetMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    setNormalHints(_minWidth, _minHeight, width, height);
}

void LinuxApplication::onSetSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    if (!_fullscreen) {
        setNormalHints(_minWidth, _minHeight, _maxWidth, _maxHeight);
        _x11.XResizeWindow(_display, _window, width, height);
        _width  = width;
        _height = height;
    } else {
        _originWidth  = width;
        _originHeight = height;
    }
}

gerium_utf8_t LinuxApplication::onGetTitle() const noexcept {
    return _title.c_str();
}

void LinuxApplication::onSetTitle(gerium_utf8_t title) noexcept {
    if (_title != title) {
        _title = title;
        _x11.XChangeProperty(_display,
                             _window,
                             NET_WM_NAME,
                             UTF8_STRING,
                             8,
                             PropModeReplace,
                             (unsigned char*) _title.data(),
                             _title.length());
        _x11.XChangeProperty(_display,
                             _window,
                             NET_WM_ICON_NAME,
                             UTF8_STRING,
                             8,
                             PropModeReplace,
                             (unsigned char*) _title.data(),
                             _title.length());
        _x11.XFlush(_display);
    }
}

void LinuxApplication::onShowCursor(bool show) noexcept {
    // TODO: At the moment I can't check, because it is not supported in wsl2
    if (show) {
        _x11.XUngrabPointer(_display, CurrentTime);
    } else {
        _x11.XGrabPointer(_display,
                          _window,
                          True,
                          ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                          GrabModeAsync,
                          GrabModeAsync,
                          _window,
                          None,
                          CurrentTime);
    }
}

gerium_float32_t LinuxApplication::onGetDensity() const noexcept {
    return _density;
}

gerium_float32_t LinuxApplication::onGetDimension(gerium_dimension_unit_t unit, gerium_float32_t value) const noexcept {
    switch (unit) {
        case GERIUM_DIMENSION_UNIT_PX:
            return value;
        case GERIUM_DIMENSION_UNIT_MM:
            return value * _dpi * kInchesPerMm;
        case GERIUM_DIMENSION_UNIT_DIP:
            return value * _density;
        case GERIUM_DIMENSION_UNIT_SP:
            return value * _scaledDensity;
        case GERIUM_DIMENSION_UNIT_PT:
            return value * _dpi * kInchesPerPt;
        case GERIUM_DIMENSION_UNIT_IN:
            return value * _dpi;
        default:
            assert(!"unreachable code");
            return 0.0f;
    }
}

void LinuxApplication::onRun() {
    if (!_window) {
        error(GERIUM_RESULT_ERROR_APPLICATION_TERMINATED);
    }
    if (_running) {
        error(GERIUM_RESULT_ERROR_APPLICATION_ALREADY_RUNNING);
    }

    _running = true;

    bool frameError = false;

    changeState(GERIUM_APPLICATION_STATE_CREATE);
    changeState(GERIUM_APPLICATION_STATE_INITIALIZE);

    showWindow();
    focusWindow();

    auto prevTime = std::chrono::high_resolution_clock::now();

    while (_running) {
        waitEvents();
        pollEvents();

        auto currentTime   = std::chrono::high_resolution_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - prevTime).count();
        if (elapsed == 0) {
            continue;
        }
        prevTime = currentTime;

        if (_running && _window && !callFrameFunc(elapsed)) {
            frameError = true;
            _running   = false;
        }
    }

    if (frameError || callbackStateFailed()) {
        error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
}

void LinuxApplication::onExit() noexcept {
    XEvent reply               = { ClientMessage };
    reply.xclient.window       = _window;
    reply.xclient.message_type = WM_PROTOCOLS;
    reply.xclient.format       = 32;
    reply.xclient.data.l[0]    = WM_DELETE_WINDOW;
    reply.xclient.data.l[1]    = CurrentTime;
    _x11.XSendEvent(_display, _window, False, NoEventMask, &reply);
}

void LinuxApplication::onShowMessage(gerium_utf8_t title, gerium_utf8_t message) noexcept {
}

bool LinuxApplication::onIsRunning() const noexcept {
    return _running;
}

void LinuxApplication::onInitImGui() {
    ImGuiIO& io            = ImGui::GetIO();
    io.BackendPlatformName = "imgui_impl_x11";
}

void LinuxApplication::onShutdownImGui() {
    ImGuiIO& io            = ImGui::GetIO();
    io.BackendPlatformName = nullptr;
}

void LinuxApplication::onNewFrameImGui() {
    ImGuiIO& io = ImGui::GetIO();

    int32_t windowWidth  = _width;
    int32_t windowHeight = _height;
    int displayWidth     = windowWidth;
    int displayHeight    = windowHeight;

    io.DisplaySize = ImVec2((float) windowWidth, (float) windowHeight);
    if (windowWidth > 0 && windowHeight > 0) {
        io.DisplayFramebufferScale = ImVec2((float) displayWidth / windowWidth, (float) displayHeight / windowHeight);
    }

    timespec currentTimespec;
    clock_gettime(CLOCK_MONOTONIC, &currentTimespec);
    double currentTime = (double) (currentTimespec.tv_sec) + (currentTimespec.tv_nsec / 1000000000.0);
    io.DeltaTime       = _imguiTime > 0.0 ? (float) (currentTime - _imguiTime) : (float) (1.0f / 60.0f);
    _imguiTime         = currentTime;
}

bool LinuxApplication::isVisible() const {
    XWindowAttributes wa;
    _x11.XGetWindowAttributes(_display, _window, &wa);
    return wa.map_state == IsViewable;
}

void LinuxApplication::showWindow() {
    if (!isVisible()) {
        _x11.XMapWindow(_display, _window);
        waitVisible();
    }
}

void LinuxApplication::focusWindow() {
    if (NET_ACTIVE_WINDOW) {
        sendWMEvent(NET_ACTIVE_WINDOW, 1, 0, 0, 0, 0);
    } else if (isVisible()) {
        _x11.XRaiseWindow(_display, _window);
        _x11.XSetInputFocus(_display, _window, RevertToParent, CurrentTime);
    }
    _x11.XFlush(_display);
}

void LinuxApplication::restoreMode() {
    if (_randr.available && _originMode) {
        auto it = std::find_if(_displays.cbegin(), _displays.cend(), [this](const auto& display) {
            return display.id == _displayId;
        });

        if (it == _displays.cend()) {
            return;
        }

        auto sr = _randr.XRRGetScreenResourcesCurrent(_display, _root);
        auto ci = _randr.XRRGetCrtcInfo(_display, sr, it->crtc);

        _randr.XRRSetCrtcConfig(
            _display, sr, it->crtc, CurrentTime, ci->x, ci->y, _originMode, ci->rotation, ci->outputs, ci->noutput);

        _randr.XRRFreeCrtcInfo(ci);
        _randr.XRRFreeScreenResources(sr);

        _originMode = {};
    }
}

void LinuxApplication::setMode() {
    if (_randr.available && _displayMode) {
        auto it = std::find_if(_displays.cbegin(), _displays.cend(), [this](const auto& display) {
            return display.id == _displayId;
        });

        if (it == _displays.cend()) {
            return;
        }

        const auto& display = *it;

        auto sr = _randr.XRRGetScreenResourcesCurrent(_display, _root);
        auto ci = _randr.XRRGetCrtcInfo(_display, sr, display.crtc);
        auto oi = _randr.XRRGetOutputInfo(_display, sr, display.output);

        size_t m = 0;
        for (; m < display.modes.size(); ++m) {
            if (display.modes[m].width == _displayMode->width && display.modes[m].height == _displayMode->height &&
                display.modes[m].refresh_rate == _displayMode->refresh_rate) {
                break;
            }
        }
        if (display.modes.size() != m) {
            auto modeId = display.modeIds[m];
            _originMode = ci->mode;

            _randr.XRRSetCrtcConfig(
                _display, sr, display.crtc, CurrentTime, ci->x, ci->y, modeId, ci->rotation, ci->outputs, ci->noutput);
        }

        _randr.XRRFreeOutputInfo(oi);
        _randr.XRRFreeCrtcInfo(ci);
        _randr.XRRFreeScreenResources(sr);
    }
}

void LinuxApplication::sendWMEvent(Atom type, long a1, long a2, long a3, long a4, long a5) {
    XEvent event               = { ClientMessage };
    event.xclient.window       = _window;
    event.xclient.format       = 32;
    event.xclient.message_type = type;
    event.xclient.data.l[0]    = a1;
    event.xclient.data.l[1]    = a2;
    event.xclient.data.l[2]    = a3;
    event.xclient.data.l[3]    = a4;
    event.xclient.data.l[4]    = a5;
    _x11.XSendEvent(_display, _root, False, SubstructureNotifyMask | SubstructureRedirectMask, &event);
}

void LinuxApplication::setNormalHints(gerium_uint16_t minWidth,
                                      gerium_uint16_t minHeight,
                                      gerium_uint16_t maxWidth,
                                      gerium_uint16_t maxHeight) {
    if (!_fullscreen) {
        XSizeHints* hints = _x11.XAllocSizeHints();

        long value;
        _x11.XGetWMNormalHints(_display, _window, hints, &value);

        hints->flags &= ~(PMinSize | PMaxSize);

        if (_styles & GERIUM_APPLICATION_STYLE_RESIZABLE_BIT) {
            if (_minWidth != kNoValue && _minHeight != kNoValue) {
                hints->flags |= PMinSize;
                hints->min_width  = _minWidth;
                hints->min_height = _minHeight;
            }
            if (_maxWidth != kNoValue && _maxHeight != kNoValue) {
                hints->flags |= PMaxSize;
                hints->max_width  = _maxWidth;
                hints->max_height = _maxHeight;
            }
        } else {
            hints->flags |= PMinSize | PMaxSize;
            hints->min_width = hints->max_width = _width;
            hints->min_height = hints->max_height = _height;
        }

        _x11.XSetWMNormalHints(_display, _window, hints);
        _x11.XFree(hints);
        _x11.XFlush(_display);
    }

    _minWidth  = minWidth;
    _minHeight = minHeight;
    _maxWidth  = maxWidth;
    _maxHeight = maxHeight;
}

void LinuxApplication::handleEvent(XEvent& event) {
    LinuxApplication* app = nullptr;
    if (_x11.XFindContext(_display, _window, _context, (XPointer*) &app) != 0) {
        return;
    }

    static bool stateChanged = false;

    switch (event.type) {
        case VisibilityNotify:
            changeState(GERIUM_APPLICATION_STATE_VISIBLE);
            break;

        case FocusIn:
            if (event.xfocus.mode != NotifyGrab && event.xfocus.mode != NotifyUngrab) {
                _active = true;
                changeState(GERIUM_APPLICATION_STATE_GOT_FOCUS);
                clearStates(_lastInputTimestamp);
            }
            break;

        case FocusOut:
            if (event.xfocus.mode != NotifyGrab && event.xfocus.mode != NotifyUngrab) {
                _active = false;
                clearStates(_lastInputTimestamp);
                changeState(GERIUM_APPLICATION_STATE_LOST_FOCUS);
            }
            break;

        case ClientMessage:
            if (event.xclient.message_type == WM_PROTOCOLS) {
                auto protocol = event.xclient.data.l[0];
                if (protocol == WM_DELETE_WINDOW) {
                    changeState(GERIUM_APPLICATION_STATE_INVISIBLE);
                    changeState(GERIUM_APPLICATION_STATE_UNINITIALIZE);
                    changeState(GERIUM_APPLICATION_STATE_DESTROY);
                    _running = false;
                } else if (protocol == NET_WM_PING) {
                    XEvent reply         = event;
                    reply.xclient.window = _root;
                    _x11.XSendEvent(_display, _root, False, SubstructureNotifyMask | SubstructureRedirectMask, &reply);
                }
            }
            break;

        case PropertyNotify:
            if (event.xproperty.atom == WM_STATE) {
                int result = WithdrawnState;
                if (auto states = getProperties<int>(WM_STATE, WM_STATE); states.size() >= 2) {
                    result = states[0];
                }
                if (result == IconicState) {
                    changeState(GERIUM_APPLICATION_STATE_MINIMIZE);
                    stateChanged = true;
                    restoreMode();
                }
            } else if (event.xproperty.atom == NET_WM_STATE) {
                auto states = getProperties<Atom>(NET_WM_STATE, XA_ATOM);
                if (states.size() == 1 && states[0] == NET_WM_STATE_FULLSCREEN) {
                    changeState(GERIUM_APPLICATION_STATE_FULLSCREEN);
                    stateChanged = true;
                    _fullscreen  = true;
                    setMode();
                } else if (states.size() == 2) {
                    std::vector maximizeStates{ NET_WM_STATE_MAXIMIZED_HORZ, NET_WM_STATE_MAXIMIZED_VERT };
                    std::sort(maximizeStates.begin(), maximizeStates.end());
                    std::sort(states.begin(), states.end());
                    if (states[0] == maximizeStates[0] && states[1] == maximizeStates[1]) {
                        changeState(GERIUM_APPLICATION_STATE_MAXIMIZE);
                        stateChanged = true;
                        _fullscreen  = false;
                        restoreMode();
                    }
                } else if (states.empty()) {
                    changeState(GERIUM_APPLICATION_STATE_NORMAL);
                    stateChanged = true;
                    _fullscreen  = false;
                    restoreMode();
                }
            }
            break;

        case ConfigureNotify:
            if (int newWidth = event.xconfigure.width, newHeight = event.xconfigure.height;
                newWidth != _width || newHeight != _height) {
                if (newWidth > 0 && newHeight > 0) {
                    _width  = newWidth;
                    _height = newHeight;
                    if (!stateChanged) {
                        _resizing       = true;
                        _lastResizeTime = std::chrono::steady_clock::now();
                        changeState(GERIUM_APPLICATION_STATE_RESIZE);
                    }
                    stateChanged = false;
                }
            }
            break;

        case GenericEvent:
            if (event.xcookie.extension == _xinput.extension) {
                switch (event.xcookie.evtype) {
                    case XI_KeyPress:
                    case XI_KeyRelease:
                        if (_x11.XGetEventData(_display, &event.xcookie)) {
                            auto keyEvent       = (XIDeviceEvent*) event.xcookie.data;
                            auto scancode       = toScanCode((ScanCode) keyEvent->detail);
                            auto press          = event.xcookie.evtype == XI_KeyPress;
                            _lastInputTimestamp = keyEvent->time;

                            if (press != isPressScancode(scancode)) {
                                XKeyEvent lookupEvent{};
                                lookupEvent.type        = KeyPress;
                                lookupEvent.serial      = keyEvent->serial;
                                lookupEvent.send_event  = keyEvent->send_event;
                                lookupEvent.display     = keyEvent->display;
                                lookupEvent.window      = keyEvent->event;
                                lookupEvent.root        = keyEvent->root;
                                lookupEvent.subwindow   = keyEvent->child;
                                lookupEvent.time        = keyEvent->time;
                                lookupEvent.x           = keyEvent->event_x;
                                lookupEvent.y           = keyEvent->event_y;
                                lookupEvent.x_root      = keyEvent->root_x;
                                lookupEvent.y_root      = keyEvent->root_y;
                                lookupEvent.state       = keyEvent->mods.locked | keyEvent->mods.base;
                                lookupEvent.keycode     = keyEvent->detail;
                                lookupEvent.same_screen = False;

                                auto keycode = toKeyCode(
                                    scancode, keyEvent->mods.base & ShiftMask, keyEvent->mods.locked & Mod2Mask);

                                gerium_event_t e{};
                                e.type              = GERIUM_EVENT_TYPE_KEYBOARD;
                                e.timestamp         = keyEvent->time;
                                e.keyboard.scancode = scancode;
                                e.keyboard.code     = keycode;
                                e.keyboard.state    = press ? GERIUM_KEY_STATE_PRESSED : GERIUM_KEY_STATE_RELEASED;

                                if (isPressScancode(GERIUM_SCANCODE_SHIFT_LEFT)) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_LSHIFT;
                                }
                                if (isPressScancode(GERIUM_SCANCODE_SHIFT_RIGHT)) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_RSHIFT;
                                }
                                if (isPressScancode(GERIUM_SCANCODE_CONTROL_LEFT)) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_LCTRL;
                                }
                                if (isPressScancode(GERIUM_SCANCODE_CONTROL_RIGHT)) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_RCTRL;
                                }
                                if (isPressScancode(GERIUM_SCANCODE_ALT_LEFT)) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_LALT;
                                }
                                if (isPressScancode(GERIUM_SCANCODE_ALT_RIGHT)) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_RALT;
                                }
                                if (isPressScancode(GERIUM_SCANCODE_META_LEFT)) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_LMETA;
                                }
                                if (isPressScancode(GERIUM_SCANCODE_META_RIGHT)) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_RMETA;
                                }
                                if (keyEvent->mods.locked & LockMask) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_CAPS_LOCK;
                                }
                                if (keyEvent->mods.locked & Mod2Mask) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_NUM_LOCK;
                                }

                                if (_ic) {
                                    Status status;
                                    char buffer[100];
                                    auto count = _x11.Xutf8LookupString(
                                        _ic, &lookupEvent, buffer, sizeof(buffer) - 1, nullptr, &status);

                                    if ((status == XLookupChars || status == XLookupBoth) && count <= 5) {
                                        buffer[count]        = '\0';
                                        e.keyboard.symbol[0] = buffer[0];
                                        e.keyboard.symbol[1] = buffer[1];
                                        e.keyboard.symbol[2] = buffer[2];
                                        e.keyboard.symbol[3] = buffer[3];
                                        e.keyboard.symbol[4] = buffer[4];
                                    }
                                } else {
                                    KeySym keysym{};
                                    _x11.XLookupString(&lookupEvent, nullptr, 0, &keysym, nullptr);
                                    if ((keysym >= 0x20 && keysym <= 0x7E) || (keysym >= 0xA0 && keysym <= 0xFF)) {
                                        e.keyboard.symbol[0] = (char) keysym;
                                    } else if (keysym == 65288) {
                                        e.keyboard.symbol[0] = '\b';
                                    } else if (keysym == 65289) {
                                        e.keyboard.symbol[0] = '\t';
                                    } else if (keysym == 65293 || keysym == 65421) {
                                        e.keyboard.symbol[0] = '\n';
                                    } else if (keysym == 65307) {
                                        e.keyboard.symbol[0] = char(0x1B);
                                    } else if (keysym == 65439 || keysym == 65535) {
                                        e.keyboard.symbol[0] = char(0x7F);
                                    } else if (keysym >= 65450 && keysym <= 65465) {
                                        e.keyboard.symbol[0] = char(keysym - 65408);
                                    }
                                }

                                setKeyState(scancode, press);
                                if (ImGui::GetCurrentContext() && !imguiHandleEvent(e) &&
                                    !ImGui::GetIO().WantCaptureKeyboard) {
                                    addEvent(e);
                                }
                            }
                            _x11.XFreeEventData(_display, &event.xcookie);
                        }
                        break;

                    case XI_ButtonPress:
                    case XI_ButtonRelease:
                        if (_x11.XGetEventData(_display, &event.xcookie)) {
                            auto motionEvent = (XIDeviceEvent*) event.xcookie.data;
                            int up           = event.xcookie.evtype == XI_ButtonRelease ? 1 : 0;

                            auto it                = _pointers.find(0);
                            const auto prevPointer = it != _pointers.end()
                                                         ? it->second
                                                         : GeriumPointer{ gerium_sint16_t(motionEvent->event_x + 0.5),
                                                                          gerium_sint16_t(motionEvent->event_y + 0.5) };

                            gerium_event_t e{};
                            e.type             = GERIUM_EVENT_TYPE_MOUSE;
                            e.timestamp        = motionEvent->time;
                            e.mouse.id         = 0;
                            e.mouse.buttons    = GERIUM_MOUSE_BUTTON_NONE;
                            e.mouse.absolute_x = gerium_sint16_t(motionEvent->event_x + 0.5);
                            e.mouse.absolute_y = gerium_sint16_t(motionEvent->event_y + 0.5);

                            switch (motionEvent->detail) {
                                case Button1:
                                    e.mouse.buttons |= gerium_mouse_button_flags_t(GERIUM_MOUSE_BUTTON_LEFT_DOWN << up);
                                    break;
                                case Button3:
                                    e.mouse.buttons |=
                                        gerium_mouse_button_flags_t(GERIUM_MOUSE_BUTTON_RIGHT_DOWN << up);
                                    break;
                                case Button2:
                                    e.mouse.buttons |=
                                        gerium_mouse_button_flags_t(GERIUM_MOUSE_BUTTON_MIDDLE_DOWN << up);
                                    break;
                            }

                            if (event.xcookie.evtype == XI_ButtonPress) {
                                constexpr gerium_float32_t scrollAmount = 1.5f;
                                switch (motionEvent->detail) {
                                    case 4:
                                        e.mouse.wheel_vertical = scrollAmount;
                                        break;
                                    case 5:
                                        e.mouse.wheel_vertical = -scrollAmount;
                                        break;
                                    case 6:
                                        e.mouse.wheel_horizontal = scrollAmount;
                                        break;
                                    case 7:
                                        e.mouse.wheel_horizontal = -scrollAmount;
                                        break;
                                }
                            }

                            _pointers[0] = { e.mouse.absolute_x, e.mouse.absolute_y, e.mouse.buttons };

                            _x11.XFreeEventData(_display, &event.xcookie);

                            if (e.mouse.buttons != prevPointer.buttions || e.mouse.wheel_vertical != 0.0f ||
                                e.mouse.wheel_horizontal != 0.0f) {
                                if (ImGui::GetCurrentContext() && !imguiHandleEvent(e) &&
                                    !ImGui::GetIO().WantCaptureMouse) {
                                    addEvent(e);
                                }
                            }
                        }
                        break;

                    case XI_Motion:
                        if (_x11.XGetEventData(_display, &event.xcookie)) {
                            auto motionEvent = (XIDeviceEvent*) event.xcookie.data;

                            auto it                = _pointers.find(0);
                            const auto prevPointer = it != _pointers.end()
                                                         ? it->second
                                                         : GeriumPointer{ gerium_sint16_t(motionEvent->event_x + 0.5),
                                                                          gerium_sint16_t(motionEvent->event_y + 0.5) };

                            gerium_event_t e{};
                            e.type                   = GERIUM_EVENT_TYPE_MOUSE;
                            e.timestamp              = motionEvent->time;
                            e.mouse.id               = 0;
                            e.mouse.buttons          = GERIUM_MOUSE_BUTTON_NONE;
                            e.mouse.absolute_x       = gerium_sint16_t(motionEvent->event_x + 0.5);
                            e.mouse.absolute_y       = gerium_sint16_t(motionEvent->event_y + 0.5);
                            e.mouse.delta_x          = e.mouse.absolute_x - prevPointer.x;
                            e.mouse.delta_y          = e.mouse.absolute_y - prevPointer.y;
                            e.mouse.raw_delta_x      = e.mouse.delta_x;
                            e.mouse.raw_delta_y      = e.mouse.delta_y;
                            e.mouse.wheel_vertical   = 0;
                            e.mouse.wheel_horizontal = 0;

                            _pointers[0] = { e.mouse.absolute_x, e.mouse.absolute_y, prevPointer.buttions };

                            _x11.XFreeEventData(_display, &event.xcookie);

                            if (e.mouse.delta_x != 0 || e.mouse.delta_y != 0) {
                                if (ImGui::GetCurrentContext() && !imguiHandleEvent(e) &&
                                    !ImGui::GetIO().WantCaptureMouse) {
                                    addEvent(e);
                                }
                            }
                        }
                        break;
                }
            }
            break;
    }
}

void LinuxApplication::pollEvents() {
    _x11.XPending(_display);

    while (QLength(_display)) {
        XEvent event;
        _x11.XNextEvent(_display, &event);
        handleEvent(event);
    }

    _x11.XFlush(_display);

    if (_resizing) {
        const auto elapsed = std::chrono::steady_clock::now() - _lastResizeTime;
        if (elapsed > std::chrono::milliseconds(500)) {
            _resizing = false;
            changeState(GERIUM_APPLICATION_STATE_RESIZED);
        }
    }
}

void LinuxApplication::waitEvents() {
    while (getBackgroundWait() && !_active) {
        waitActive();
    }
}

void LinuxApplication::waitVisible() {
    XEvent dummy;
    while (!_x11.XCheckTypedWindowEvent(_display, _window, VisibilityNotify, &dummy)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    changeState(GERIUM_APPLICATION_STATE_VISIBLE);
}

void LinuxApplication::waitActive() {
    XEvent dummy;
    while (!_x11.XCheckTypedWindowEvent(_display, _window, FocusIn, &dummy)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    _active = true;
    changeState(GERIUM_APPLICATION_STATE_GOT_FOCUS);
    clearStates(_lastInputTimestamp);
}

bool LinuxApplication::imguiHandleEvent(const gerium_event_t& event) const {
    auto& io = ImGui::GetIO();

    if (!isShowCursor() && !io.WantCaptureMouse) {
        ImGui::GetIO().AddFocusEvent(false);
    }

    switch (event.type) {
        case GERIUM_EVENT_TYPE_KEYBOARD: {
            io.AddKeyEvent(ImGuiMod_Ctrl, (event.keyboard.modifiers & GERIUM_KEY_MOD_CTRL) != 0);
            io.AddKeyEvent(ImGuiMod_Shift, (event.keyboard.modifiers & GERIUM_KEY_MOD_SHIFT) != 0);
            io.AddKeyEvent(ImGuiMod_Alt, (event.keyboard.modifiers & GERIUM_KEY_MOD_ALT) != 0);
            io.AddKeyEvent(ImGuiMod_Super, (event.keyboard.modifiers & GERIUM_KEY_MOD_META) != 0);

            auto key = toImguiKey(event.keyboard.scancode);
            if (key != ImGuiKey_None) {
                io.AddKeyEvent(key, event.keyboard.state == GERIUM_KEY_STATE_PRESSED);
                io.SetKeyEventNativeData(key, event.keyboard.code, event.keyboard.scancode);
            }

            break;
        }
        case GERIUM_EVENT_TYPE_MOUSE: {
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            if (event.mouse.delta_x != 0 || event.mouse.delta_y != 0) {
                io.AddMousePosEvent(event.mouse.absolute_x, event.mouse.absolute_y);
            }
            if (event.mouse.buttons & GERIUM_MOUSE_BUTTON_LEFT_DOWN) {
                io.AddMouseButtonEvent(0, true);
            } else if (event.mouse.buttons & GERIUM_MOUSE_BUTTON_LEFT_UP) {
                io.AddMouseButtonEvent(0, false);
            }
            if (event.mouse.buttons & GERIUM_MOUSE_BUTTON_RIGHT_DOWN) {
                io.AddMouseButtonEvent(1, true);
            } else if (event.mouse.buttons & GERIUM_MOUSE_BUTTON_RIGHT_UP) {
                io.AddMouseButtonEvent(1, false);
            }
            if (event.mouse.buttons & GERIUM_MOUSE_BUTTON_MIDDLE_DOWN) {
                io.AddMouseButtonEvent(2, true);
            } else if (event.mouse.buttons & GERIUM_MOUSE_BUTTON_MIDDLE_UP) {
                io.AddMouseButtonEvent(2, false);
            }
            if (event.mouse.wheel_vertical != 0.0f || event.mouse.wheel_horizontal != 0.0f) {
                io.AddMouseWheelEvent(event.mouse.wheel_horizontal, event.mouse.wheel_vertical);
            }
            break;
        }
    }

    if (io.WantTextInput && event.type == GERIUM_EVENT_TYPE_KEYBOARD &&
        event.keyboard.state == GERIUM_KEY_STATE_RELEASED) {
        if (event.keyboard.symbol[0] != '\0') {
            io.AddInputCharactersUTF8(event.keyboard.symbol);
        }
        return true;
    }

    return false;
}

void LinuxApplication::calcDensity() {
    gerium_float32_t scale         = 0.0f;
    gerium_float32_t scaledDensity = 1.0f;
    if (auto sdlScale = getenv("SDL_VIDEO_X11_SCALING_FACTOR")) {
        auto value = std::atof(sdlScale);
        if (value >= 1.0f && value <= 10.0f) {
            scale = value;
        }
    }
    if (scale == 0.0f) {
        _x11.XrmInitialize();
        if (auto resourceManager = _x11.XResourceManagerString(_display)) {
            XrmValue value;
            char* type;
            auto db = _x11.XrmGetStringDatabase(resourceManager);
            if (_x11.XrmGetResource(db, "Xft.dpi", "String", &type, &value)) {
                if (value.addr && type && strcmp(type, "String") == 0) {
                    auto dpi = gerium_float32_t(std::atoi(value.addr));
                    scale    = dpi / 96.0f;
                }
            }
            _x11.XrmDestroyDatabase(db);
        }
    }
    if (scale == 0.0f) {
        if (auto gdkScale = getenv("GDK_SCALE")) {
            auto value = std::atof(gdkScale);
            if (value >= 1.0f && value <= 10.0f) {
                scale = value;
            }
        }
    }
    if (scale == 0.0f) {
        scale = 1.0f;
    }

    if (auto gdkScaledDensity = getenv("GDK_DPI_SCALE")) {
        auto value    = std::atof(gdkScaledDensity);
        scaledDensity = value;
    }

    _dpi           = scale * 96.0f;
    _density       = scale;
    _scaledDensity = scaledDensity;
}

void LinuxApplication::error(gerium_result_t error, const std::string_view message, bool throwError) {
    if (_errorCode != Success) {
        char buffer[512];
        _x11.XGetErrorText(_display, _errorCode, buffer, sizeof(buffer));
        _logger->print(GERIUM_LOGGER_LEVEL_ERROR, [&message, buffer](auto& stream) {
            stream << message << ": " << buffer;
        });
        if (throwError) {
            Object::error(error);
        }
    }
}

void LinuxApplication::acquirereErrorHandler() {
    assert(!_errorHandler);
    _errorCode    = Success;
    _errorHandler = _x11.XSetErrorHandler(errorHandler);
}

void LinuxApplication::releaseErrorHandler() {
    _x11.XSync(_display, False);
    _x11.XSetErrorHandler(_errorHandler);
    _errorHandler = nullptr;
}

int LinuxApplication::errorHandler(Display* display, XErrorEvent* event) {
    if (_display == display) {
        _errorCode = event->error_code;
    }
    return 0;
}

void LinuxApplication::destroyIm(XIM im, XPointer clientData, XPointer callData) {
    auto app = (LinuxApplication*) clientData;
    app->_im = nullptr;
}

void LinuxApplication::destroyIc(XIM im, XPointer clientData, XPointer callData) {
    auto app = (LinuxApplication*) clientData;
    app->_ic = nullptr;
}

LinuxApplication::X11Table::X11Table() {
    dll = dlopen("libX11.so.6", RTLD_GLOBAL | RTLD_LAZY);
    if (!dll) {
        dll = dlopen("libX11.so", RTLD_GLOBAL | RTLD_LAZY);
    } else if (!dll) {
        dll = dlopen("libX11-6.so", RTLD_GLOBAL | RTLD_LAZY);
    }
    if (!dll) {
        error(GERIUM_RESULT_ERROR_APPLICATION_CREATE);
    }

    XAllocClassHint        = (PFN_XAllocClassHint) dlsym(dll, "XAllocClassHint");
    XAllocSizeHints        = (PFN_XAllocSizeHints) dlsym(dll, "XAllocSizeHints");
    XAllocWMHints          = (PFN_XAllocWMHints) dlsym(dll, "XAllocWMHints");
    XChangeProperty        = (PFN_XChangeProperty) dlsym(dll, "XChangeProperty");
    XCheckTypedWindowEvent = (PFN_XCheckTypedWindowEvent) dlsym(dll, "XCheckTypedWindowEvent");
    XCloseDisplay          = (PFN_XCloseDisplay) dlsym(dll, "XCloseDisplay");
    XCloseIM               = (PFN_XCloseIM) dlsym(dll, "XCloseIM");
    XCreateColormap        = (PFN_XCreateColormap) dlsym(dll, "XCreateColormap");
    XCreateIC              = (PFN_XCreateIC) dlsym(dll, "XCreateIC");
    XCreateWindow          = (PFN_XCreateWindow) dlsym(dll, "XCreateWindow");
    XDeleteContext         = (PFN_XDeleteContext) dlsym(dll, "XDeleteContext");
    XDeleteProperty        = (PFN_XDeleteProperty) dlsym(dll, "XDeleteProperty");
    XDestroyIC             = (PFN_XDestroyIC) dlsym(dll, "XDestroyIC");
    XDestroyWindow         = (PFN_XDestroyWindow) dlsym(dll, "XDestroyWindow");
    XFindContext           = (PFN_XFindContext) dlsym(dll, "XFindContext");
    XFlush                 = (PFN_XFlush) dlsym(dll, "XFlush");
    XFree                  = (PFN_XFree) dlsym(dll, "XFree");
    XFreeColormap          = (PFN_XFreeColormap) dlsym(dll, "XFreeColormap");
    XFreeEventData         = (PFN_XFreeEventData) dlsym(dll, "XFreeEventData");
    XGetErrorText          = (PFN_XGetErrorText) dlsym(dll, "XGetErrorText");
    XGetEventData          = (PFN_XGetEventData) dlsym(dll, "XGetEventData");
    XGetICValues           = (PFN_XGetICValues) dlsym(dll, "XGetICValues");
    XGetIMValues           = (PFN_XGetIMValues) dlsym(dll, "XGetIMValues");
    XGetWindowAttributes   = (PFN_XGetWindowAttributes) dlsym(dll, "XGetWindowAttributes");
    XGetWindowProperty     = (PFN_XGetWindowProperty) dlsym(dll, "XGetWindowProperty");
    XGetWMNormalHints      = (PFN_XGetWMNormalHints) dlsym(dll, "XGetWMNormalHints");
    XGrabPointer           = (PFN_XGrabPointer) dlsym(dll, "XGrabPointer");
    XInternAtom            = (PFN_XInternAtom) dlsym(dll, "XInternAtom");
    XLookupString          = (PFN_XLookupString) dlsym(dll, "XLookupString");
    XMapRaised             = (PFN_XMapRaised) dlsym(dll, "XMapRaised");
    XMapWindow             = (PFN_XMapWindow) dlsym(dll, "XMapWindow");
    XNextEvent             = (PFN_XNextEvent) dlsym(dll, "XNextEvent");
    XOpenDisplay           = (PFN_XOpenDisplay) dlsym(dll, "XOpenDisplay");
    XOpenIM                = (PFN_XOpenIM) dlsym(dll, "XOpenIM");
    XPending               = (PFN_XPending) dlsym(dll, "XPending");
    XQueryExtension        = (PFN_XQueryExtension) dlsym(dll, "XQueryExtension");
    XRaiseWindow           = (PFN_XRaiseWindow) dlsym(dll, "XRaiseWindow");
    XResizeWindow          = (PFN_XResizeWindow) dlsym(dll, "XResizeWindow");
    XResourceManagerString = (PFN_XResourceManagerString) dlsym(dll, "XResourceManagerString");
    XrmDestroyDatabase     = (PFN_XrmDestroyDatabase) dlsym(dll, "XrmDestroyDatabase");
    XrmGetResource         = (PFN_XrmGetResource) dlsym(dll, "XrmGetResource");
    XrmGetStringDatabase   = (PFN_XrmGetStringDatabase) dlsym(dll, "XrmGetStringDatabase");
    XrmInitialize          = (PFN_XrmInitialize) dlsym(dll, "XrmInitialize");
    XrmUniqueQuark         = (PFN_XrmUniqueQuark) dlsym(dll, "XrmUniqueQuark");
    XSaveContext           = (PFN_XSaveContext) dlsym(dll, "XSaveContext");
    XSelectInput           = (PFN_XSelectInput) dlsym(dll, "XSelectInput");
    XSendEvent             = (PFN_XSendEvent) dlsym(dll, "XSendEvent");
    XSetClassHint          = (PFN_XSetClassHint) dlsym(dll, "XSetClassHint");
    XSetErrorHandler       = (PFN_XSetErrorHandler) dlsym(dll, "XSetErrorHandler");
    XSetIMValues           = (PFN_XSetIMValues) dlsym(dll, "XSetIMValues");
    XSetInputFocus         = (PFN_XSetInputFocus) dlsym(dll, "XSetInputFocus");
    XSetWMHints            = (PFN_XSetWMHints) dlsym(dll, "XSetWMHints");
    XSetWMNormalHints      = (PFN_XSetWMNormalHints) dlsym(dll, "XSetWMNormalHints");
    XSetWMProtocols        = (PFN_XSetWMProtocols) dlsym(dll, "XSetWMProtocols");
    XSync                  = (PFN_XSync) dlsym(dll, "XSync");
    XUngrabPointer         = (PFN_XUngrabPointer) dlsym(dll, "XUngrabPointer");
    XUnmapWindow           = (PFN_XUnmapWindow) dlsym(dll, "XUnmapWindow");
    Xutf8LookupString      = (PFN_Xutf8LookupString) dlsym(dll, "Xutf8LookupString");
}

LinuxApplication::X11Table::~X11Table() {
    if (dll) {
        dlclose(dll);
    }
}

LinuxApplication::X11XCBTable::X11XCBTable() {
    dll = dlopen("libX11-xcb.so.1", RTLD_GLOBAL | RTLD_LAZY);
    if (!dll) {
        dll = dlopen("libX11-xcb.so", RTLD_GLOBAL | RTLD_LAZY);
    } else if (!dll) {
        dll = dlopen("libX11-xcb-1.so", RTLD_GLOBAL | RTLD_LAZY);
    }
    if (!dll) {
        error(GERIUM_RESULT_ERROR_APPLICATION_CREATE);
    }

    XGetXCBConnection = (PFN_XGetXCBConnection) dlsym(dll, "XGetXCBConnection");
}

LinuxApplication::X11XCBTable::~X11XCBTable() {
    if (dll) {
        dlclose(dll);
    }
}

LinuxApplication::XInput2Table::XInput2Table() {
    dll = dlopen("libXi.so.6", RTLD_GLOBAL | RTLD_LAZY);
    if (!dll) {
        dll = dlopen("libXi.so", RTLD_GLOBAL | RTLD_LAZY);
    } else if (!dll) {
        dll = dlopen("libXi-6.so", RTLD_GLOBAL | RTLD_LAZY);
    }
    if (!dll) {
        error(GERIUM_RESULT_ERROR_APPLICATION_CREATE);
    }

    XIQueryVersion = (PFN_XIQueryVersion) dlsym(dll, "XIQueryVersion");
    XISelectEvents = (PFN_XISelectEvents) dlsym(dll, "XISelectEvents");
}

LinuxApplication::XInput2Table::~XInput2Table() {
    if (dll) {
        dlclose(dll);
    }
}

void LinuxApplication::XInput2Table::setup(X11Table& x11) {
    if (!x11.XQueryExtension(_display, "XInputExtension", &extension, &eventBase, &errorBase)) {
        error(GERIUM_RESULT_ERROR_APPLICATION_CREATE);
    }

    major = 2;
    minor = 0;
    if (XIQueryVersion(_display, &major, &minor) != Success) {
        error(GERIUM_RESULT_ERROR_APPLICATION_CREATE);
    }
}

LinuxApplication::XineramaTable::XineramaTable() {
    dll = dlopen("libXinerama.so.1", RTLD_GLOBAL | RTLD_LAZY);
    if (!dll) {
        dll = dlopen("libXinerama.so", RTLD_GLOBAL | RTLD_LAZY);
    } else if (!dll) {
        dll = dlopen("libXinerama-1.so", RTLD_GLOBAL | RTLD_LAZY);
    }

    if (dll) {
        XineramaIsActive       = (PFN_XineramaIsActive) dlsym(dll, "XineramaIsActive");
        XineramaQueryExtension = (PFN_XineramaQueryExtension) dlsym(dll, "XineramaQueryExtension");
        XineramaQueryScreens   = (PFN_XineramaQueryScreens) dlsym(dll, "XineramaQueryScreens");
    }
}

LinuxApplication::XineramaTable::~XineramaTable() {
    if (dll) {
        dlclose(dll);
    }
}

void LinuxApplication::XineramaTable::setup() {
    if (dll) {
        available = XineramaQueryExtension(_display, &major, &minor) && XineramaIsActive(_display);
    }
}

LinuxApplication::XRandrTable::XRandrTable() {
    dll = dlopen("libXrandr.so.2", RTLD_GLOBAL | RTLD_LAZY);
    if (!dll) {
        dll = dlopen("libXrandr.so", RTLD_GLOBAL | RTLD_LAZY);
    } else if (!dll) {
        dll = dlopen("libXrandr-2.so", RTLD_GLOBAL | RTLD_LAZY);
    }

    if (dll) {
        XRRAllocGamma                = (PFN_XRRAllocGamma) dlsym(dll, "XRRAllocGamma");
        XRRFreeCrtcInfo              = (PFN_XRRFreeCrtcInfo) dlsym(dll, "XRRFreeCrtcInfo");
        XRRFreeGamma                 = (PFN_XRRFreeGamma) dlsym(dll, "XRRFreeGamma");
        XRRFreeOutputInfo            = (PFN_XRRFreeOutputInfo) dlsym(dll, "XRRFreeOutputInfo");
        XRRFreeScreenResources       = (PFN_XRRFreeScreenResources) dlsym(dll, "XRRFreeScreenResources");
        XRRGetCrtcGamma              = (PFN_XRRGetCrtcGamma) dlsym(dll, "XRRGetCrtcGamma");
        XRRGetCrtcGammaSize          = (PFN_XRRGetCrtcGammaSize) dlsym(dll, "XRRGetCrtcGammaSize");
        XRRGetCrtcInfo               = (PFN_XRRGetCrtcInfo) dlsym(dll, "XRRGetCrtcInfo");
        XRRGetOutputInfo             = (PFN_XRRGetOutputInfo) dlsym(dll, "XRRGetOutputInfo");
        XRRGetOutputPrimary          = (PFN_XRRGetOutputPrimary) dlsym(dll, "XRRGetOutputPrimary");
        XRRGetScreenResourcesCurrent = (PFN_XRRGetScreenResourcesCurrent) dlsym(dll, "XRRGetScreenResourcesCurrent");
        XRRQueryExtension            = (PFN_XRRQueryExtension) dlsym(dll, "XRRQueryExtension");
        XRRQueryVersion              = (PFN_XRRQueryVersion) dlsym(dll, "XRRQueryVersion");
        XRRSelectInput               = (PFN_XRRSelectInput) dlsym(dll, "XRRSelectInput");
        XRRSetCrtcConfig             = (PFN_XRRSetCrtcConfig) dlsym(dll, "XRRSetCrtcConfig");
        XRRSetCrtcGamma              = (PFN_XRRSetCrtcGamma) dlsym(dll, "XRRSetCrtcGamma");
        XRRUpdateConfiguration       = (PFN_XRRUpdateConfiguration) dlsym(dll, "XRRUpdateConfiguration");
    }
}

LinuxApplication::XRandrTable::~XRandrTable() {
    if (dll) {
        dlclose(dll);
    }
}

void LinuxApplication::XRandrTable::setup(Window root) {
    if (dll) {
        if (XRRQueryExtension(_display, &eventBase, &errorBase)) {
            if (XRRQueryVersion(_display, &major, &minor)) {
                available = major > 1 || minor >= 3;
            }
        }
    }

    if (available) {
        auto sr = XRRGetScreenResourcesCurrent(_display, root);
        if (!sr->ncrtc || !XRRGetCrtcGammaSize(_display, sr->crtcs[0])) {
            gammaBroken = true;
        }
        if (!sr->ncrtc) {
            available = false;
        }
        XRRFreeScreenResources(sr);
    }

    if (available) {
        XRRSelectInput(_display, root, RROutputChangeNotifyMask);
    }
}

int LinuxApplication::_errorCode{};

Display* LinuxApplication::_display{};

} // namespace gerium::linux

gerium_result_t gerium_application_create(gerium_utf8_t title,
                                          gerium_uint32_t width,
                                          gerium_uint32_t height,
                                          gerium_application_t* application) {
    using namespace gerium;
    using namespace gerium::linux;
    return Object::create<LinuxApplication>(*application, title, width, height);
}
