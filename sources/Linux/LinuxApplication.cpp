#include "LinuxApplication.hpp"
#include "LinuxScanCodes.hpp"

namespace gerium::linux {

LinuxApplication::LinuxApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) :
    _logger(Logger::create("gerium:application:x11")) {
    _display = XOpenDisplay(nullptr);

    if (!_display) {
        error(GERIUM_RESULT_ERROR_NO_DISPLAY);
    }

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
    wa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask |
                    ButtonReleaseMask | ExposureMask | FocusChangeMask | VisibilityChangeMask | EnterWindowMask |
                    LeaveWindowMask | PropertyChangeMask;

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
}

LinuxApplication::~LinuxApplication() {
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
    /*
    if (!displays) {
        displayCount = 0;
    } else {
        _modes.clear();
    }

    auto iter             = xcb_setup_roots_iterator(_setup);
    gerium_uint32_t count = 0;

    for (; iter.rem && count < displayCount; xcb_screen_next(&iter), ++count) {
        if (displays) {
            auto reply = xcb_randr_get_screen_resources_current_reply(
                _connection, xcb_randr_get_screen_resources_current(_connection, iter.data->root), nullptr);

            auto reply2 = xcb_randr_get_screen_info_reply(
                _connection, xcb_randr_get_screen_info(_connection, iter.data->root), nullptr);

            auto modeIter  = xcb_randr_get_screen_resources_current_modes_iterator(reply);
            auto modeCount = 0;
            for (; modeIter.rem; xcb_randr_mode_info_next(&modeIter), ++modeCount) {
                _modes.push_back({ modeIter.data->width, modeIter.data->height, 0 });
            }

            displays[count].id          = count;
            displays[count].name        = "Unknown";
            displays[count].gpu_name    = "Unknown";
            displays[count].device_name = "Unknown";
            displays[count].device_name = "Unknown";
            displays[count].mode_count  = modeCount;
        }
    }

    for (gerium_uint32_t i = 0, offset = 0; i < count; ++i) {
        displays[i].modes = &_modes[offset];
        offset += displays[i].mode_count;
    }

    displayCount = count;
    */
}

bool LinuxApplication::onIsFullscreen() const noexcept {
    return false;
}

void LinuxApplication::onFullscreen(bool fullscreen, gerium_uint32_t displayId, const gerium_display_mode_t* mode) {
    /*_isFullscreen = fullscreen;

    auto event            = (xcb_client_message_event_t*) calloc(sizeof(xcb_client_message_event_t), 1);
    event->response_type  = XCB_CLIENT_MESSAGE;
    event->window         = _window;
    event->format         = 32;
    event->type           = _wmStateAtom;
    event->data.data32[0] = fullscreen ? 1 : 0;
    event->data.data32[1] = _wmStateFullscreen;
    event->data.data32[2] = 0;
    event->data.data32[3] = 1;
    event->data.data32[4] = 0;
    xcb_send_event(_connection,
                   false,
                   _screen->root,
                   XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                   (const char*) event);

    if (!fullscreen) {
        decorate(_styles);
        onSetSize(_width, _height);
    }

    xcb_unmap_window(_connection, _window);
    xcb_map_window(_connection, _window);*/
}

gerium_application_style_flags_t LinuxApplication::onGetStyle() const noexcept {
    return GERIUM_APPLICATION_STYLE_NONE_BIT;
}

void LinuxApplication::onSetStyle(gerium_application_style_flags_t style) noexcept {
}

void LinuxApplication::onGetMinSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    // if (width) {
    //     *width = _minWidth;
    // }
    // if (height) {
    //     *height = _minHeight;
    // }
}

void LinuxApplication::onGetMaxSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    // if (width) {
    //     *width = _maxWidth;
    // }
    // if (height) {
    //     *height = _maxHeight;
    // }
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
    // xcb_size_hints_t hints{};
    // xcb_icccm_size_hints_set_min_size(&hints, width, height);
    // xcb_icccm_size_hints_set_max_size(&hints, _maxWidth, _maxHeight);
    // xcb_icccm_set_wm_size_hints(_connection, _window, XCB_ATOM_WM_NORMAL_HINTS, &hints);
}

void LinuxApplication::onSetMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    // xcb_size_hints_t hints{};
    // xcb_icccm_size_hints_set_min_size(&hints, _minWidth, _minHeight);
    // xcb_icccm_size_hints_set_max_size(&hints, width, height);
    // xcb_icccm_set_wm_size_hints(_connection, _window, XCB_ATOM_WM_NORMAL_HINTS, &hints);
}

void LinuxApplication::onSetSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    // uint32_t values[] = { width, height };
    // xcb_configure_window(_connection, _window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
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
    // if (show) {
    //     xcb_xfixes_show_cursor(_connection, _window);
    // } else {
    //     xcb_xfixes_hide_cursor(_connection, _window);
    // }
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

        if (_running && !callFrameFunc(elapsed)) {
            frameError = true;
            _running   = false;
        }
    }

    if (frameError || callbackStateFailed()) {
        error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }

    /* if (_running) {
        error(GERIUM_RESULT_ERROR_APPLICATION_ALREADY_RUNNING);
    }
    _running = true;

    static bool resizing  = false;
    static bool prevState = false;
    bool frameError       = false;

    changeState(GERIUM_APPLICATION_STATE_CREATE);
    changeState(GERIUM_APPLICATION_STATE_INITIALIZE);

    xcb_map_window(_connection, _window);
    xcb_flush(_connection);

    while (_running) {
        if (xcb_connection_has_error(_connection)) {
            break;
        }

        while (auto event = xcb_poll_for_event(_connection)) {
            switch (event->response_type & ~0x80) {
                case XCB_CONFIGURE_NOTIFY: {
                    auto newWidth  = ((xcb_configure_notify_event_t*) event)->width;
                    auto newHeight = ((xcb_configure_notify_event_t*) event)->height;
                    if (newWidth != _width || newHeight != _height) {
                        _width  = newWidth;
                        _height = newHeight;
                        if (!prevState) {
                            changeState(GERIUM_APPLICATION_STATE_RESIZE);
                            resizing = true;
                        }
                        prevState = false;
                    }
                    break;
                }

                case XCB_PROPERTY_NOTIFY:
                    if (((xcb_property_notify_event_t*) event)->atom == _wmStateAtom) {
                        auto states = getValueAtoms(_wmStateAtom);

                        if (states.size() == 1 && states[0] == _wmStateFullscreen) {
                            changeState(GERIUM_APPLICATION_STATE_FULLSCREEN);
                        } else if (states.size() == 2) {
                            std::vector maximizeStates{ _wmStateMaximizedHorz, _wmStateMaximizedVert };
                            std::sort(maximizeStates.begin(), maximizeStates.end());
                            std::sort(states.begin(), states.end());
                            if (states[0] == maximizeStates[0] && states[1] == maximizeStates[1]) {
                                changeState(GERIUM_APPLICATION_STATE_MAXIMIZE);
                            }
                        } else if (states.size() == 1 && states[0] == _wmStateHidden) {
                            changeState(GERIUM_APPLICATION_STATE_MINIMIZE);
                        } else if (states.empty()) {
                            changeState(GERIUM_APPLICATION_STATE_NORMAL);
                        }
                        prevState = true;
                    }
                    break;

                case XCB_FOCUS_IN:
                    changeState(GERIUM_APPLICATION_STATE_GOT_FOCUS);
                    break;

                case XCB_FOCUS_OUT:
                    changeState(GERIUM_APPLICATION_STATE_LOST_FOCUS);
                    break;

                case XCB_VISIBILITY_NOTIFY:
                    changeState(GERIUM_APPLICATION_STATE_VISIBLE);
                    break;

                case XCB_ENTER_NOTIFY:
                case XCB_LEAVE_NOTIFY:
                    switch (((xcb_enter_notify_event_t*) event)->mode) {
                        case XCB_NOTIFY_MODE_NORMAL:
                            if (resizing) {
                                changeState(GERIUM_APPLICATION_STATE_RESIZED);
                                resizing = false;
                            }
                            break;
                    }
                    break;

                case XCB_CLIENT_MESSAGE:
                    if (((xcb_client_message_event_t*) event)->data.data32[0] == _closeReply->atom) {
                        changeState(GERIUM_APPLICATION_STATE_INVISIBLE);
                        changeState(GERIUM_APPLICATION_STATE_UNINITIALIZE);
                        changeState(GERIUM_APPLICATION_STATE_DESTROY);
                        _running = false;
                    }
                    break;

                case XCB_GE_GENERIC:
                    switch (((xcb_ge_event_t*) event)->event_type) {
                        case XCB_INPUT_KEY_PRESS:
                        case XCB_INPUT_KEY_RELEASE:
                            auto press    = ((xcb_ge_event_t*) event)->event_type == XCB_INPUT_KEY_PRESS;
                            auto key      = (xcb_input_key_press_event_t*) event;
                            auto scancode = toScanCode((ScanCode) key->detail);

                            if (isPressScancode(scancode) != press) {
                                gerium_event_t e{};
                                e.type              = GERIUM_EVENT_TYPE_KEYBOARD;
                                e.timestamp         = key->time;
                                e.keyboard.scancode = scancode;
                                e.keyboard.code     = GERIUM_KEY_CODE_UNKNOWN;
                                e.keyboard.state    = press ? GERIUM_KEY_STATE_PRESSED : GERIUM_KEY_STATE_RELEASED;
                                if (key->mods.locked & XCB_MOD_MASK_LOCK) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_CAPS_LOCK;
                                }
                                if (key->mods.locked & XCB_MOD_MASK_2) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_NUM_LOCK;
                                }
                                if (key->mods.locked & XCB_MOD_MASK_4) {
                                    e.keyboard.modifiers |= GERIUM_KEY_MOD_SCROLL_LOCK;
                                }
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
                                // e.keyboard.symbol[5];

                                setKeyState(scancode, press);
                                addEvent(e);
                            }
                            break;
                    }
                    break;
            }
            free(event);
        }

        if (_running && !callFrameFunc(16)) {
            frameError = true;
            exit();
        }
    }

    xcb_disconnect(_connection);
    _window = 0;

    if (frameError || callbackStateFailed()) {
        error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }*/
}

void LinuxApplication::onExit() noexcept {
    // xcb_destroy_window(_connection, _window);
}

bool LinuxApplication::onIsRunning() const noexcept {
    return true; // _running;
}

void LinuxApplication::onInitImGui() {
}

void LinuxApplication::onShutdownImGui() {
}

void LinuxApplication::onNewFrameImGui() {
}

void LinuxApplication::decorate(gerium_application_style_flags_t styles) {
    /* _styles = styles;

    if (_isFullscreen) {
        return;
    }

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
    constexpr uint32_t MWM_DECOR_RESIZEH  = 1L << 2;
    constexpr uint32_t MWM_DECOR_TITLE    = 1L << 3;
    constexpr uint32_t MWM_DECOR_MENU     = 1L << 4;
    constexpr uint32_t MWM_DECOR_MINIMIZE = 1L << 5;
    constexpr uint32_t MWM_DECOR_MAXIMIZE = 1L << 6;

    struct MotifWmHints {
        uint32_t flags;
        uint32_t functions;
        uint32_t decorations;
        int32_t inputMode;
        uint32_t status;
    };

    MotifWmHints hints{};
    hints.decorations = MWM_DECOR_BORDER | MWM_DECOR_TITLE;
    hints.functions   = MWM_FUNC_MOVE | MWM_FUNC_CLOSE;
    hints.flags       = MWM_HINTS_DECORATIONS | MWM_HINTS_FUNCTIONS;
    hints.inputMode   = 0;
    hints.status      = 0;

    if (styles & GERIUM_APPLICATION_STYLE_RESIZABLE_BIT) {
        hints.decorations |= MWM_DECOR_RESIZEH;
        hints.functions |= MWM_DECOR_RESIZEH;
    }

    if (styles & GERIUM_APPLICATION_STYLE_MINIMIZABLE_BIT) {
        hints.decorations |= MWM_DECOR_MINIMIZE;
        hints.functions |= MWM_FUNC_MINIMIZE;
    }

    if (styles & GERIUM_APPLICATION_STYLE_MAXIMIZABLE_BIT) {
        hints.decorations |= MWM_DECOR_MAXIMIZE;
        hints.functions |= MWM_FUNC_MAXIMIZE;
    }

    xcb_change_property(
        _connection, XCB_PROP_MODE_REPLACE, _window, _wmWindowAtom, XCB_ATOM_ATOM, 32, 1, &_wmWindowNormalAtom);
    xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, _wmHits, XCB_ATOM_ATOM, 32, 5, &hints); */
}

xcb_atom_t LinuxApplication::getAtom(std::string_view name, bool onlyIfExists) const {
    // auto cookie = xcb_intern_atom(_connection, onlyIfExists ? 1 : 0, name.length(), name.data());
    // auto replay = xcb_intern_atom_reply(_connection, cookie, nullptr);
    // auto atom   = replay->atom;
    // free(replay);
    // return atom;
    return {};
}

std::vector<xcb_atom_t> LinuxApplication::getValueAtoms(xcb_atom_t atom) const {
    // auto reply = xcb_get_property_reply(
    //     _connection, xcb_get_property(_connection, 0, _window, atom, XCB_ATOM_ATOM, 0, 32), nullptr);
    // std::vector<xcb_atom_t> results;
    // if (reply->length && reply->format == 32) {
    //     auto values = (xcb_atom_t*) xcb_get_property_value(reply);
    //     for (uint32_t i = 0; i < reply->length; ++i) {
    //         results.push_back(values[i]);
    //     }
    // }
    // free(reply);
    // return results;
    return {};
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

void LinuxApplication::handleEvent(const XEvent& event) {
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
            }
            break;

        case FocusOut:
            if (event.xfocus.mode != NotifyGrab && event.xfocus.mode != NotifyUngrab) {
                _active = false;
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
                } else if (states.size() == 2) {
                    std::vector maximizeStates{ NET_WM_STATE_MAXIMIZED_HORZ, NET_WM_STATE_MAXIMIZED_VERT };
                    std::sort(maximizeStates.begin(), maximizeStates.end());
                    std::sort(states.begin(), states.end());
                    if (states[0] == maximizeStates[0] && states[1] == maximizeStates[1]) {
                        changeState(GERIUM_APPLICATION_STATE_MAXIMIZE);
                        stateChanged = true;
                    }
                } else if (states.empty()) {
                    changeState(GERIUM_APPLICATION_STATE_NORMAL);
                    stateChanged = true;
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
