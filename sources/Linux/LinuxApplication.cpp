#include "LinuxApplication.hpp"
#include "LinuxScanCodes.hpp"

namespace gerium::linux {

LinuxApplication::LinuxApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) :
    _logger(Logger::create("gerium:application:x11")) {
    _display = XOpenDisplay(nullptr);

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
    _colormap = XCreateColormap(_display, _root, visual, AllocNone);
    _context  = XUniqueContext();

    UTF8_STRING                 = XInternAtom(_display, "UTF8_STRING", False);
    WM_PROTOCOLS                = XInternAtom(_display, "WM_PROTOCOLS", False);
    WM_STATE                    = XInternAtom(_display, "WM_STATE", False);
    WM_DELETE_WINDOW            = XInternAtom(_display, "WM_DELETE_WINDOW", False);
    NET_ACTIVE_WINDOW           = XInternAtom(_display, "_NET_ACTIVE_WINDOW", False);
    NET_WM_NAME                 = XInternAtom(_display, "_NET_WM_NAME", False);
    NET_WM_ICON_NAME            = XInternAtom(_display, "_NET_WM_ICON_NAME", False);
    NET_WM_ICON                 = XInternAtom(_display, "_NET_WM_ICON", False);
    NET_WM_PID                  = XInternAtom(_display, "_NET_WM_PID", False);
    NET_WM_PING                 = XInternAtom(_display, "_NET_WM_PING", False);
    NET_WM_WINDOW_TYPE          = XInternAtom(_display, "_NET_WM_WINDOW_TYPE", False);
    NET_WM_WINDOW_TYPE_NORMAL   = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    NET_WM_STATE                = XInternAtom(_display, "_NET_WM_STATE", False);
    NET_WM_STATE_ABOVE          = XInternAtom(_display, "_NET_WM_STATE_ABOVE", False);
    NET_WM_STATE_FULLSCREEN     = XInternAtom(_display, "_NET_WM_STATE_FULLSCREEN", False);
    NET_WM_STATE_MAXIMIZED_VERT = XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    NET_WM_STATE_MAXIMIZED_HORZ = XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    NET_WM_FULLSCREEN_MONITORS  = XInternAtom(_display, "_NET_WM_FULLSCREEN_MONITORS ", False);
    MOTIF_WM_HINTS              = XInternAtom(_display, "_MOTIF_WM_HINTS", False);

    XSetWindowAttributes wa{};
    wa.colormap   = _colormap;
    wa.event_mask = StructureNotifyMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | ExposureMask |
                    FocusChangeMask | VisibilityChangeMask | EnterWindowMask | LeaveWindowMask | PropertyChangeMask;

    {
        ErrorGuard error(this);
        _window = XCreateWindow(_display,
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
    error(GERIUM_RESULT_ERROR_UNKNOWN, "create X11 window failed", true); // TODO add error

    XSaveContext(_display, _window, _context, (XPointer) this);

    Atom protocols[] = { WM_DELETE_WINDOW, NET_WM_PING };
    XSetWMProtocols(_display, _window, protocols, std::size(protocols));

    const long pid = getpid();
    XChangeProperty(_display, _window, NET_WM_PID, XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &pid, 1);

    if (NET_WM_WINDOW_TYPE && NET_WM_WINDOW_TYPE_NORMAL) {
        auto type = NET_WM_WINDOW_TYPE_NORMAL;
        XChangeProperty(_display, _window, NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (unsigned char*) &type, 1);
    }

    auto wmHints = XAllocWMHints();
    if (!wmHints) {
        error(GERIUM_RESULT_ERROR_OUT_OF_MEMORY);
    }
    wmHints->flags         = StateHint;
    wmHints->initial_state = NormalState;
    XSetWMHints(_display, _window, wmHints);
    XFree(wmHints);

    auto sizeHints = XAllocSizeHints();
    if (!sizeHints) {
        error(GERIUM_RESULT_ERROR_OUT_OF_MEMORY);
    }
    sizeHints->flags |= PWinGravity;
    sizeHints->win_gravity = StaticGravity;
    XSetWMNormalHints(_display, _window, sizeHints);
    XFree(sizeHints);

    auto classHint = XAllocClassHint();
    if (!classHint) {
        error(GERIUM_RESULT_ERROR_OUT_OF_MEMORY);
    }
    auto name            = strlen(title) ? (char*) title : (char*) "gerium";
    classHint->res_name  = name;
    classHint->res_class = name;
    XSetClassHint(_display, _window, classHint);
    XFree(classHint);

    onSetTitle(title);

    _im = XOpenIM(_display, nullptr, nullptr, nullptr);
    if (_im) {
        bool found        = false;
        XIMStyles* styles = NULL;

        if (!XGetIMValues(_im, XNQueryInputStyle, &styles, NULL)) {
            for (unsigned int i = 0; i < styles->count_styles; i++) {
                if (styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing)) {
                    found = true;
                    break;
                }
            }
            XFree(styles);
        }

        if (!found) {
            XCloseIM(_im);
            _im = nullptr;
        }
    }

    if (_im) {
        XIMCallback callbackIm;
        callbackIm.callback    = destroyIm;
        callbackIm.client_data = (XPointer) this;
        XSetIMValues(_im, XNDestroyCallback, &callbackIm, nullptr);

        XIMCallback callbackIc;
        callbackIc.callback    = destroyIc;
        callbackIc.client_data = (XPointer) this;

        _ic = XCreateIC(_im,
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
            XGetWindowAttributes(_display, _window, &attribs);

            unsigned long filter = 0;
            if (XGetICValues(_ic, XNFilterEvents, &filter, NULL) == NULL) {
                XSelectInput(_display, _window, attribs.your_event_mask | filter);
            }
        }
    }

    int queryEvent;
    int queryError;
    if (!XQueryExtension(_display, "XInputExtension", &_xinput.extension, &queryEvent, &queryError)) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }

    int major = 2, minor = 0;
    if (_xinput.XIQueryVersion(_display, &major, &minor) != Success) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }

    unsigned char masks[XIMaskLen(XI_LASTEVENT)]{};
    XIEventMask xiMask{};
    xiMask.deviceid = XIAllMasterDevices;
    xiMask.mask_len = sizeof(masks);
    xiMask.mask     = masks;
    XISetMask(xiMask.mask, XI_KeyPress);
    XISetMask(xiMask.mask, XI_KeyRelease);

    _xinput.XISelectEvents(_display, _window, &xiMask, 1);
    XSync(_display, False);
}

LinuxApplication::~LinuxApplication() {
    if (_ic) {
        XDestroyIC(_ic);
    }
    if (_im) {
        XCloseIM(_im);
    }

    if (_display) {
        if (_window) {
            XDeleteContext(_display, _window, _context);
            XUnmapWindow(_display, _window);
            XDestroyWindow(_display, _window);
        }
        if (_colormap) {
            XFreeColormap(_display, _colormap);
        }
        XFlush(_display);
        XCloseDisplay(_display);
    }
}

xcb_connection_t* LinuxApplication::connection() const noexcept {
    return XGetXCBConnection(_display);
}

uint32_t LinuxApplication::window() const noexcept {
    return _window;
}

gerium_runtime_platform_t LinuxApplication::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_LINUX;
}

void LinuxApplication::onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const {
    displayCount = 0;
}

bool LinuxApplication::onIsFullscreen() const noexcept {
    return _fullscreen;
}

void LinuxApplication::onFullscreen(bool fullscreen, gerium_uint32_t displayId, const gerium_display_mode_t* mode) {
    if (!isVisible()) {
        XMapRaised(_display, _window);
        waitVisible();
    }

    if (NET_WM_STATE && NET_WM_STATE_FULLSCREEN) {
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

        XFlush(_display);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        onSetStyle(_styles);
        XFlush(_display);
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

        XChangeProperty(_display,
                        _window,
                        MOTIF_WM_HINTS,
                        MOTIF_WM_HINTS,
                        32,
                        PropModeReplace,
                        (unsigned char*) &hints,
                        sizeof(hints) / sizeof(long));
    } else {
        XChangeProperty(_display,
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
        XResizeWindow(_display, _window, width, height);
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
        XChangeProperty(_display,
                        _window,
                        NET_WM_NAME,
                        UTF8_STRING,
                        8,
                        PropModeReplace,
                        (unsigned char*) _title.data(),
                        _title.length());
        XChangeProperty(_display,
                        _window,
                        NET_WM_ICON_NAME,
                        UTF8_STRING,
                        8,
                        PropModeReplace,
                        (unsigned char*) _title.data(),
                        _title.length());
        XFlush(_display);
    }
}

void LinuxApplication::onShowCursor(bool show) noexcept {
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
    XSendEvent(_display, _window, False, NoEventMask, &reply);
}

bool LinuxApplication::onIsRunning() const noexcept {
    return _running;
}

void LinuxApplication::onInitImGui() {
}

void LinuxApplication::onShutdownImGui() {
}

void LinuxApplication::onNewFrameImGui() {
}

bool LinuxApplication::isVisible() const {
    XWindowAttributes wa;
    XGetWindowAttributes(_display, _window, &wa);
    return wa.map_state == IsViewable;
}

void LinuxApplication::showWindow() {
    if (!isVisible()) {
        XMapWindow(_display, _window);
        waitVisible();
    }
}

void LinuxApplication::focusWindow() {
    if (NET_ACTIVE_WINDOW) {
        sendWMEvent(NET_ACTIVE_WINDOW, 1, 0, 0, 0, 0);
    } else if (isVisible()) {
        XRaiseWindow(_display, _window);
        XSetInputFocus(_display, _window, RevertToParent, CurrentTime);
    }
    XFlush(_display);
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
    XSendEvent(_display, _root, False, SubstructureNotifyMask | SubstructureRedirectMask, &event);
}

void LinuxApplication::setNormalHints(gerium_uint16_t minWidth,
                                      gerium_uint16_t minHeight,
                                      gerium_uint16_t maxWidth,
                                      gerium_uint16_t maxHeight) {
    if (!_fullscreen) {
        XSizeHints* hints = XAllocSizeHints();

        long value;
        XGetWMNormalHints(_display, _window, hints, &value);

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

        XSetWMNormalHints(_display, _window, hints);
        XFree(hints);
        XFlush(_display);
    }

    _minWidth  = minWidth;
    _minHeight = minHeight;
    _maxWidth  = maxWidth;
    _maxHeight = maxHeight;
}

void LinuxApplication::handleEvent(XEvent& event) {
    LinuxApplication* app = nullptr;
    if (XFindContext(_display, _window, _context, (XPointer*) &app) != 0) {
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
                    XSendEvent(_display, _root, False, SubstructureNotifyMask | SubstructureRedirectMask, &reply);
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
                }
            } else if (event.xproperty.atom == NET_WM_STATE) {
                auto states = getProperties<Atom>(NET_WM_STATE, XA_ATOM);
                if (states.size() == 1 && states[0] == NET_WM_STATE_FULLSCREEN) {
                    changeState(GERIUM_APPLICATION_STATE_FULLSCREEN);
                    stateChanged = true;
                    _fullscreen  = true;
                } else if (states.size() == 2) {
                    std::vector maximizeStates{ NET_WM_STATE_MAXIMIZED_HORZ, NET_WM_STATE_MAXIMIZED_VERT };
                    std::sort(maximizeStates.begin(), maximizeStates.end());
                    std::sort(states.begin(), states.end());
                    if (states[0] == maximizeStates[0] && states[1] == maximizeStates[1]) {
                        changeState(GERIUM_APPLICATION_STATE_MAXIMIZE);
                        stateChanged = true;
                        _fullscreen  = false;
                    }
                } else if (states.empty()) {
                    changeState(GERIUM_APPLICATION_STATE_NORMAL);
                    stateChanged = true;
                    _fullscreen  = false;
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
                    case XI_KeyRelease: {
                        if (XGetEventData(_display, &event.xcookie)) {
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
                                    auto count = Xutf8LookupString(
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
                                    XLookupString(&lookupEvent, nullptr, 0, &keysym, nullptr);
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
                                addEvent(e);
                            }
                            XFreeEventData(_display, &event.xcookie);
                        }
                        break;
                    }
                }
            }
            break;
    }
}

void LinuxApplication::pollEvents() {
    XPending(_display);

    while (QLength(_display)) {
        XEvent event;
        XNextEvent(_display, &event);
        handleEvent(event);
    }

    XFlush(_display);

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
    while (!XCheckTypedWindowEvent(_display, _window, VisibilityNotify, &dummy)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    changeState(GERIUM_APPLICATION_STATE_VISIBLE);
}

void LinuxApplication::waitActive() {
    XEvent dummy;
    while (!XCheckTypedWindowEvent(_display, _window, FocusIn, &dummy)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    _active = true;
    changeState(GERIUM_APPLICATION_STATE_GOT_FOCUS);
    clearStates(_lastInputTimestamp);
}

void LinuxApplication::error(gerium_result_t error, const std::string_view message, bool throwError) {
    if (_errorCode != Success) {
        char buffer[512];
        XGetErrorText(_display, _errorCode, buffer, sizeof(buffer));
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
    _errorHandler = XSetErrorHandler(errorHandler);
}

void LinuxApplication::releaseErrorHandler() {
    XSync(_display, False);
    XSetErrorHandler(_errorHandler);
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

LinuxApplication::XInput2Table::XInput2Table() {
    dll = dlopen("libXi.so.6", RTLD_GLOBAL | RTLD_LAZY);
    if (!dll) {
        dll = dlopen("libXi.so", RTLD_GLOBAL | RTLD_LAZY);
    } else if (!dll) {
        dll = dlopen("libXi.so.6", RTLD_GLOBAL | RTLD_LAZY);
    }
    if (!dll) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }

    XIQueryVersion = (PFN_XIQueryVersion) dlsym(dll, "XIQueryVersion");
    XISelectEvents = (PFN_XISelectEvents) dlsym(dll, "XISelectEvents");
}

LinuxApplication::XInput2Table::~XInput2Table() {
    if (dll) {
        dlclose(dll);
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
