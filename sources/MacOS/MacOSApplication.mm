#include "MacOSApplication.hpp"
#include "MacOSScanCodes.hpp"

#include <imgui_impl_osx.h>

#import <GameController/GameController.h>
#import <IOKit/graphics/IOGraphicsLib.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>

@interface WindowViewController : NSViewController <NSApplicationDelegate, NSWindowDelegate, MTKViewDelegate> {
@public
    gerium::macos::MacOSApplication* application;
}

- (void)handleKeyboardConnected:(NSNotification*)notification;

- (void)handleKeyboardDisconnected:(NSNotification*)notification;

- (void)handleMouseConnected:(NSNotification*)notification;

- (void)handleMouseDisconnected:(NSNotification*)notification;

- (void)subscribeKeyboardEvents:(GCKeyboard*)keyboard;

- (void)unsubscribeKeyboardEvents:(GCKeyboard*)keyboard;

- (void)subscribeMouseEvents:(GCMouse*)mouse;

- (void)unsubscribeMouseEvents:(GCMouse*)mouse;

- (void)addKeyboardEvent:(NSEvent*)event pressed:(BOOL)down;

- (void)addMouseEvent:(NSEvent*)event;

- (void)initializeDevices;

@property(strong, nonatomic) NSWindow* window;

@property(strong, nonatomic) NSTrackingArea* area;

@property(nonatomic) bool imguiFoucus;

@property(nonatomic) gerium_key_mod_flags_t modifiers;

@property(nonatomic) gerium_mouse_event_t lastMouseEvent;

@end

@implementation WindowViewController

@synthesize modifiers;

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size {
    application->changeState(GERIUM_APPLICATION_STATE_RESIZE);

    auto frame = [view frame];
    frame.size = size;

    NSTrackingArea* newArea = [[NSTrackingArea alloc]
        initWithRect:frame
             options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow
               owner:view
            userInfo:nil];

    [view removeTrackingArea:self.area];
    [view addTrackingArea:newArea];

    self.area = newArea;
}

- (void)drawInMTKView:(nonnull MTKView*)view {
    if (auto& io = ImGui::GetIO(); io.WantCaptureKeyboard && !self.imguiFoucus) {
        self.imguiFoucus = true;
        application->clearEvents();
    } else if (!io.WantCaptureKeyboard && self.imguiFoucus) {
        self.imguiFoucus = false;
    }
    application->frame();
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
    application->changeState(GERIUM_APPLICATION_STATE_CREATE);
    application->changeState(GERIUM_APPLICATION_STATE_INITIALIZE);
    if (application->isStartedFullscreen()) {
        application->fullscreen(true);
    }
    if (!application->isFullscreen()) {
        application->changeState(GERIUM_APPLICATION_STATE_NORMAL);
    }

    self.imguiFoucus    = false;
    self.lastMouseEvent = {};

    [self initializeDevices];
}

- (void)applicationWillTerminate:(NSNotification*)notification {
    if ([NSApp occlusionState] & NSApplicationOcclusionStateVisible) {
        application->changeState(GERIUM_APPLICATION_STATE_INVISIBLE);
    }
    application->changeState(GERIUM_APPLICATION_STATE_UNINITIALIZE);
    application->changeState(GERIUM_APPLICATION_STATE_DESTROY);
}

- (void)applicationDidBecomeActive:(NSNotification*)notification {
    application->changeState(GERIUM_APPLICATION_STATE_GOT_FOCUS);

    MTKView* view = ((__bridge MTKView*) application->getView());
    view.paused   = NO;
}

- (void)applicationDidResignActive:(NSNotification*)notification {
    application->changeState(GERIUM_APPLICATION_STATE_LOST_FOCUS);

    if (application->getBackgroundWait()) {
        MTKView* view = ((__bridge MTKView*) application->getView());
        view.paused   = YES;
    }
}

- (void)applicationDidChangeOcclusionState:(NSNotification*)notification {
    if ([NSApp occlusionState] & NSApplicationOcclusionStateVisible) {
        application->changeState(GERIUM_APPLICATION_STATE_VISIBLE);
    } else {
        application->changeState(GERIUM_APPLICATION_STATE_INVISIBLE);
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return YES;
}

- (void)windowDidEndLiveResize:(NSNotification*)notification {
    application->changeState(GERIUM_APPLICATION_STATE_RESIZED);
}

- (void)windowDidMiniaturize:(NSNotification*)notification {
    application->changeState(GERIUM_APPLICATION_STATE_MINIMIZE);
    application->changeState(GERIUM_APPLICATION_STATE_INVISIBLE);
}

- (void)windowDidDeminiaturize:(NSNotification*)notification {
    application->changeState(GERIUM_APPLICATION_STATE_NORMAL);
    if ([NSApp occlusionState] & NSApplicationOcclusionStateVisible) {
        application->changeState(GERIUM_APPLICATION_STATE_VISIBLE);
    }
}

- (void)windowDidEnterFullScreen:(NSNotification*)notification {
    application->changeState(GERIUM_APPLICATION_STATE_FULLSCREEN);
}

- (void)windowDidExitFullScreen:(NSNotification*)notification {
    application->changeState(GERIUM_APPLICATION_STATE_NORMAL);
}

- (void)keyDown:(NSEvent*)event {
    [self addKeyboardEvent:event pressed:TRUE];
}

- (void)keyUp:(NSEvent*)event {
    [self addKeyboardEvent:event pressed:FALSE];
}

- (void)mouseDown:(NSEvent*)event {
    [self addMouseEvent:event];
}

- (void)mouseUp:(NSEvent*)event {
    [self addMouseEvent:event];
}

- (void)rightMouseDown:(NSEvent*)event {
    [self addMouseEvent:event];
}

- (void)rightMouseUp:(NSEvent*)event {
    [self addMouseEvent:event];
}

- (void)otherMouseDown:(NSEvent*)event {
    [self addMouseEvent:event];
}

- (void)otherMouseUp:(NSEvent*)event {
    [self addMouseEvent:event];
}

- (void)mouseMoved:(NSEvent*)event {
    [self addMouseEvent:event];
}

- (void)mouseDragged:(NSEvent*)event {
    [self addMouseEvent:event];
}

- (void)rightMouseDragged:(NSEvent*)event {
    [self addMouseEvent:event];
}

- (void)otherMouseDragged:(NSEvent*)event {
    [self addMouseEvent:event];
}

- (void)scrollWheel:(NSEvent*)event {
    [self addMouseEvent:event];
}

- (void)addKeyboardEvent:(NSEvent*)event pressed:(BOOL)down {
    const auto flags = [event modifierFlags];
    auto mods        = GERIUM_KEY_MOD_NONE;
    if (application->isPressed(GERIUM_SCANCODE_SHIFT_LEFT)) {
        mods |= GERIUM_KEY_MOD_LSHIFT;
    }
    if (application->isPressed(GERIUM_SCANCODE_SHIFT_RIGHT)) {
        mods |= GERIUM_KEY_MOD_RSHIFT;
    }
    if (application->isPressed(GERIUM_SCANCODE_CONTROL_LEFT)) {
        mods |= GERIUM_KEY_MOD_LCTRL;
    }
    if (application->isPressed(GERIUM_SCANCODE_CONTROL_RIGHT)) {
        mods |= GERIUM_KEY_MOD_RCTRL;
    }
    if (application->isPressed(GERIUM_SCANCODE_ALT_LEFT)) {
        mods |= GERIUM_KEY_MOD_LALT;
    }
    if (application->isPressed(GERIUM_SCANCODE_ALT_RIGHT)) {
        mods |= GERIUM_KEY_MOD_RALT;
    }
    if (application->isPressed(GERIUM_SCANCODE_META_LEFT)) {
        mods |= GERIUM_KEY_MOD_LMETA;
    }
    if (application->isPressed(GERIUM_SCANCODE_META_RIGHT)) {
        mods |= GERIUM_KEY_MOD_RMETA;
    }
    if (flags & NSEventModifierFlagCapsLock) {
        mods |= GERIUM_KEY_MOD_CAPS_LOCK;
    }
    if (flags & NSEventModifierFlagNumericPad) {
        mods |= GERIUM_KEY_MOD_NUM_LOCK;
    }

    const auto [scancode, keycode] = gerium::macos::toScanCode([event keyCode], mods);

    gerium_event_t newEvent{};
    newEvent.type               = GERIUM_EVENT_TYPE_KEYBOARD;
    newEvent.timestamp          = application->ticks();
    newEvent.keyboard.scancode  = scancode;
    newEvent.keyboard.code      = keycode;
    newEvent.keyboard.state     = down ? GERIUM_KEY_STATE_PRESSED : GERIUM_KEY_STATE_RELEASED;
    newEvent.keyboard.modifiers = mods;

    if ([[event characters] length] < 5) {
        const auto symbol           = [[event characters] UTF8String];
        newEvent.keyboard.symbol[0] = symbol[0];
        newEvent.keyboard.symbol[1] = symbol[1];
        newEvent.keyboard.symbol[2] = symbol[2];
        newEvent.keyboard.symbol[3] = symbol[3];
    }
    application->sendEvent(newEvent);

    modifiers = mods;
}

- (void)addMouseEvent:(NSEvent*)event {
    auto& io = ImGui::GetIO();

    if (io.WantCaptureMouse) {
        return;
    }

    MTKView* view = ((__bridge MTKView*) application->getView());

    NSPoint pos = event.locationInWindow;
    if (event.window == nil) {
        pos = [[view window] convertPointFromScreen:pos];
    }

    pos = [view convertPoint:pos fromView:nil];
    if ([view isFlipped]) {
        pos = NSMakePoint(pos.x, pos.y);
    } else {
        pos = NSMakePoint(pos.x, view.bounds.size.height - pos.y);
    }

    auto buttons = GERIUM_MOUSE_BUTTON_NONE;
    auto wheelX  = 0.0;
    auto wheelY  = 0.0;

    if (event.type == NSEventTypeLeftMouseDown) {
        buttons = GERIUM_MOUSE_BUTTON_LEFT_DOWN;
    } else if (event.type == NSEventTypeRightMouseDown) {
        buttons = GERIUM_MOUSE_BUTTON_RIGHT_UP;
    } else if (event.type == NSEventTypeOtherMouseDown) {
        buttons = GERIUM_MOUSE_BUTTON_MIDDLE_DOWN;
    } else if (event.type == NSEventTypeLeftMouseUp) {
        buttons = GERIUM_MOUSE_BUTTON_LEFT_UP;
    } else if (event.type == NSEventTypeRightMouseUp) {
        buttons = GERIUM_MOUSE_BUTTON_RIGHT_UP;
    } else if (event.type == NSEventTypeOtherMouseUp) {
        buttons = GERIUM_MOUSE_BUTTON_LEFT_UP;
    } else if (event.type == NSEventTypeScrollWheel && event.phase != NSEventPhaseCancelled) {
        if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6) {
            wheelX = [event scrollingDeltaX];
            wheelY = [event scrollingDeltaY];
            if ([event hasPreciseScrollingDeltas]) {
                wheelX *= 0.01;
                wheelY *= 0.01;
            }
        } else {
            wheelX = [event deltaX] * 0.1;
            wheelY = [event deltaY] * 0.1;
        }
    }

    gerium_event_t newEvent{};
    newEvent.type                   = GERIUM_EVENT_TYPE_MOUSE;
    newEvent.timestamp              = application->ticks();
    newEvent.mouse.id               = 0;
    newEvent.mouse.buttons          = buttons;
    newEvent.mouse.absolute_x       = gerium_sint16_t(pos.x * application->scale());
    newEvent.mouse.absolute_y       = gerium_sint16_t(pos.y * application->scale());
    newEvent.mouse.delta_x          = newEvent.mouse.absolute_x - self.lastMouseEvent.absolute_x;
    newEvent.mouse.delta_y          = newEvent.mouse.absolute_y - self.lastMouseEvent.absolute_y;
    newEvent.mouse.raw_delta_x      = newEvent.mouse.delta_x;
    newEvent.mouse.raw_delta_y      = newEvent.mouse.delta_y;
    newEvent.mouse.wheel_vertical   = wheelY;
    newEvent.mouse.wheel_horizontal = wheelX;

    application->sendEvent(newEvent);

    self.lastMouseEvent = newEvent.mouse;
}

- (void)initializeDevices {
    modifiers = GERIUM_KEY_MOD_NONE;

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(handleKeyboardConnected:)
                                                 name:GCKeyboardDidConnectNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(handleKeyboardDisconnected:)
                                                 name:GCKeyboardDidDisconnectNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(handleMouseConnected:)
                                                 name:GCMouseDidConnectNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(handleMouseDisconnected::)
                                                 name:GCMouseDidDisconnectNotification
                                               object:nil];
}

- (void)handleKeyboardConnected:(NSNotification*)notification {
    GCKeyboard* keyboard = notification.object;
    [self subscribeKeyboardEvents:keyboard];
}

- (void)handleKeyboardDisconnected:(NSNotification*)notification {
    GCKeyboard* keyboard = notification.object;
    [self unsubscribeKeyboardEvents:keyboard];
}

- (void)handleMouseConnected:(NSNotification*)notification {
    GCMouse* mouse = notification.object;
    [self subscribeMouseEvents:mouse];
}

- (void)handleMouseDisconnected:(NSNotification*)notification {
    GCMouse* mouse = notification.object;
    [self unsubscribeMouseEvents:mouse];
}

- (void)subscribeKeyboardEvents:(GCKeyboard*)keyboard {
    keyboard.keyboardInput.keyChangedHandler =
        ^(GCKeyboardInput* keyboardInput, GCControllerButtonInput* key, GCKeyCode keyCode, BOOL pressed) {
            gerium_event_t newEvent{};
            newEvent.type               = GERIUM_EVENT_TYPE_KEYBOARD;
            newEvent.timestamp          = application->ticks();
            newEvent.keyboard.state     = pressed ? GERIUM_KEY_STATE_PRESSED : GERIUM_KEY_STATE_RELEASED;
            newEvent.keyboard.modifiers = modifiers;

            auto newModifiers = GERIUM_KEY_MOD_NONE;

            if (keyCode == GCKeyCodeLeftControl) {
                newEvent.keyboard.scancode = GERIUM_SCANCODE_CONTROL_LEFT;
                newEvent.keyboard.code     = GERIUM_KEY_CODE_CONTROL_LEFT;
                newModifiers               = GERIUM_KEY_MOD_LCTRL;
            } else if (keyCode == GCKeyCodeLeftShift) {
                newEvent.keyboard.scancode = GERIUM_SCANCODE_SHIFT_LEFT;
                newEvent.keyboard.code     = GERIUM_KEY_CODE_SHIFT_LEFT;
                newModifiers               = GERIUM_KEY_MOD_LSHIFT;
            } else if (keyCode == GCKeyCodeLeftAlt) {
                newEvent.keyboard.scancode = GERIUM_SCANCODE_ALT_LEFT;
                newEvent.keyboard.code     = GERIUM_KEY_CODE_ALT_LEFT;
                newModifiers               = GERIUM_KEY_MOD_LALT;
            } else if (keyCode == GCKeyCodeLeftGUI) {
                newEvent.keyboard.scancode = GERIUM_SCANCODE_META_LEFT;
                newEvent.keyboard.code     = GERIUM_KEY_CODE_META_LEFT;
                newModifiers               = GERIUM_KEY_MOD_LMETA;
            } else if (keyCode == GCKeyCodeRightControl) {
                newEvent.keyboard.scancode = GERIUM_SCANCODE_CONTROL_RIGHT;
                newEvent.keyboard.code     = GERIUM_KEY_CODE_CONTROL_RIGHT;
                newModifiers               = GERIUM_KEY_MOD_RCTRL;
            } else if (keyCode == GCKeyCodeRightShift) {
                newEvent.keyboard.scancode = GERIUM_SCANCODE_SHIFT_RIGHT;
                newEvent.keyboard.code     = GERIUM_KEY_CODE_SHIFT_RIGHT;
                newModifiers               = GERIUM_KEY_MOD_RSHIFT;
            } else if (keyCode == GCKeyCodeRightAlt) {
                newEvent.keyboard.scancode = GERIUM_SCANCODE_ALT_RIGHT;
                newEvent.keyboard.code     = GERIUM_KEY_CODE_ALT_RIGHT;
                newModifiers               = GERIUM_KEY_MOD_RALT;
            } else if (keyCode == GCKeyCodeRightGUI) {
                newEvent.keyboard.scancode = GERIUM_SCANCODE_META_RIGHT;
                newEvent.keyboard.code     = GERIUM_KEY_CODE_META_RIGHT;
                newModifiers               = GERIUM_KEY_MOD_RMETA;
            } else {
                return;
            }
            application->sendEvent(newEvent);

            if (pressed) {
                modifiers |= newModifiers;
            } else {
                modifiers ^= newModifiers;
            }
        };
}

- (void)unsubscribeKeyboardEvents:(GCKeyboard*)keyboard {
    keyboard.keyboardInput.keyChangedHandler = nil;
}

- (void)subscribeMouseEvents:(GCMouse*)mouse {
    mouse.mouseInput.mouseMovedHandler = ^(GCMouseInput* mouseInput, float deltaX, float deltaY) {
        if (application->isHideCursor()) {
            gerium_event_t newEvent{};
            newEvent.type              = GERIUM_EVENT_TYPE_MOUSE;
            newEvent.timestamp         = application->ticks();
            newEvent.mouse.id          = 0;
            newEvent.mouse.absolute_x  = self.lastMouseEvent.absolute_x;
            newEvent.mouse.absolute_y  = self.lastMouseEvent.absolute_y;
            newEvent.mouse.delta_x     = gerium_sint16_t(deltaX * application->scale());
            newEvent.mouse.delta_y     = gerium_sint16_t(deltaY * application->scale());
            newEvent.mouse.raw_delta_x = newEvent.mouse.delta_x;
            newEvent.mouse.raw_delta_y = newEvent.mouse.delta_y;

            application->sendEvent(newEvent);

            self.lastMouseEvent = newEvent.mouse;

            MTKView* view         = ((__bridge MTKView*) application->getView());
            NSRect frame          = [self.window convertRectToScreen:[view frame]];
            NSPoint mouseLocation = [NSEvent mouseLocation];

            if (!NSPointInRect(mouseLocation, frame)) {
                mouseLocation.x = MIN(MAX(mouseLocation.x, NSMinX(frame) + 1), NSMaxX(frame) - 1);
                mouseLocation.y = MIN(MAX(mouseLocation.y, NSMinY(frame) + 1), NSMaxY(frame) - 1);
                auto height     = [NSScreen mainScreen].frame.size.height;
                mouseLocation.y = height - mouseLocation.y;
                CGWarpMouseCursorPosition(mouseLocation);
            }

            ImGui::GetIO().AddFocusEvent(false);
        }
    };
}

- (void)unsubscribeMouseEvents:(GCMouse*)mouse {
    mouse.mouseInput.mouseMovedHandler = nil;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

@end

namespace gerium::macos {

MacOSApplication::MacOSApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) {
    @try {
        float scale = [NSScreen mainScreen].backingScaleFactor;
        width /= scale;
        height /= scale;

        NSRect frame = NSMakeRect(0, 0, width, height);

        WindowViewController* viewController = [WindowViewController new];
        viewController->application          = this;

        MTKView* view         = [[MTKView alloc] initWithFrame:frame];
        view.delegate         = viewController;
        view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

        viewController.area = [[NSTrackingArea alloc]
            initWithRect:frame
                 options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow
                   owner:view
                userInfo:nil];

        [view addTrackingArea:viewController.area];

        NSWindow* window =
            [[NSWindow alloc] initWithContentRect:frame
                                        styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                                                   NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable)
                                          backing:NSBackingStoreBuffered
                                            defer:NO];
        window.delegate = viewController;
        window.title    = [NSString stringWithUTF8String:title];

        [window.contentView addSubview:view];
        [window center];
        [window orderFrontRegardless];

        [view setNextResponder:viewController];

        viewController.window = window;
        _scale                = window.backingScaleFactor;
        _invScale             = 1.0f / _scale;
        _viewController       = CFRetain((__bridge void*) viewController);
        _view                 = CFRetain((__bridge void*) view);

    } @catch (NSException*) {
        error(GERIUM_RESULT_ERROR_UNKNOWN);
    }
}

const CAMetalLayer* MacOSApplication::layer() const noexcept {
    MTKView* view = ((__bridge MTKView*) _view);
    return (CAMetalLayer*) view.layer;
}

void MacOSApplication::changeState(gerium_application_state_t newState) {
    if (newState != _prevState || newState == GERIUM_APPLICATION_STATE_RESIZE) {
        _prevState = newState;
        if (!callStateFunc(newState)) {
            error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
        }
    }
}

void MacOSApplication::frame() {
    auto currentTime = getCurrentTime();

    const std::chrono::duration<float, std::milli> delta = currentTime - _prevTime;
    const auto elapsed                                   = delta.count();

    if (elapsed == 0.0f) {
        return;
    }
    _prevTime = currentTime;

    if (!callFrameFunc(elapsed)) {
        error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
}

bool MacOSApplication::isStartedFullscreen() const noexcept {
    return _startFullscreen;
}

bool MacOSApplication::isFullscreen() const noexcept {
    return onIsFullscreen();
}

bool MacOSApplication::isHideCursor() const noexcept {
    return !isShowCursor();
}

void MacOSApplication::restoreWindow() noexcept {
    constexpr auto val = std::numeric_limits<gerium_uint16_t>::max();

    if (_newMinWidth != val) {
        onSetMinSize(_newMinWidth, _newMinHeight);
        _newMinWidth  = val;
        _newMinHeight = val;
    }
    if (_newMaxWidth != val) {
        onSetMaxSize(_newMaxWidth, _newMaxHeight);
        _newMaxWidth  = val;
        _newMaxHeight = val;
    }
    if (_newWidth != val) {
        onSetSize(_newWidth, _newHeight);
        _newWidth  = val;
        _newHeight = val;
    }

    onSetStyle(_styles);
}

void MacOSApplication::fullscreen(bool fullscreen) noexcept {
    onFullscreen(fullscreen, _display, _mode.has_value() ? &_mode.value() : nullptr);
}

gerium_uint16_t MacOSApplication::getPixelSize(gerium_uint16_t x) const noexcept {
    return gerium_uint16_t(x * _scale);
}

float MacOSApplication::getDeviceSize(gerium_uint16_t x) const noexcept {
    return x * _invScale;
}

float MacOSApplication::titlebarHeight() const noexcept {
    WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
    NSRect frame                     = controller.window.frame;
    return frame.size.height - [controller.window contentRectForFrameRect:frame].size.height;
}

const void* MacOSApplication::getView() const noexcept {
    return _view;
}

bool MacOSApplication::isPressed(gerium_scancode_t scancode) const noexcept {
    return isPressScancode(scancode);
}

void MacOSApplication::setPressed(gerium_scancode_t scancode, bool pressed) noexcept {
    setKeyState(scancode, pressed);
}

void MacOSApplication::sendEvent(const gerium_event_t& event) noexcept {
    if (event.type == GERIUM_EVENT_TYPE_KEYBOARD) {
        const auto pressed     = event.keyboard.state == GERIUM_KEY_STATE_PRESSED;
        const auto prevPressed = isPressScancode(event.keyboard.scancode);
        if (pressed == prevPressed) {
            return;
        }
        setKeyState(event.keyboard.scancode, pressed);
    }
    if (auto& io = ImGui::GetIO(); !io.WantCaptureKeyboard) {
        addEvent(event);
    }
}

void MacOSApplication::clearEvents() noexcept {
    clearStates(ticks());
}

float MacOSApplication::scale() const noexcept {
    return _scale;
}

gerium_uint64_t MacOSApplication::ticks() noexcept {
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (gerium_uint8_t) now.tv_sec * 1'000'000'000LL + now.tv_nsec;
}

gerium_runtime_platform_t MacOSApplication::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_MAC_OS;
}

void MacOSApplication::onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const {
    std::vector<CGDirectDisplayID> activeDisplays;
    activeDisplays.resize(32);
    CGGetActiveDisplayList(32, activeDisplays.data(), &displayCount);
    if (displays) {
        _modes.clear();
        _displayNames.clear();

        gerium_uint32_t index = 0;
        enumDisplays(activeDisplays, displayCount, true, index, displays);
        enumDisplays(activeDisplays, displayCount, false, index, displays);

        for (gerium_uint32_t i = 0, offset = 0; i < displayCount; ++i) {
            displays[i].device_name = _displayNames[i * 3].data();
            displays[i].gpu_name    = _displayNames[i * 3 + 1].data();
            displays[i].name        = _displayNames[i * 3 + 2].data();
            displays[i].modes       = _modes.data() + offset;
            offset += displays[i].mode_count;
        }
    }
}

bool MacOSApplication::onIsFullscreen() const noexcept {
    if (_running) {
        WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
        return [controller.window styleMask] & NSWindowStyleMaskFullScreen;
    } else {
        return _startFullscreen;
    }
}

void MacOSApplication::onFullscreen(bool fullscreen, gerium_uint32_t displayId, const gerium_display_mode_t* mode) {
    if (_running) {
        if (fullscreen != onIsFullscreen()) {
            WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
            if (fullscreen) {
                controller.window.minSize = NSMakeSize(0, 0);
                controller.window.maxSize = NSMakeSize(FLT_MAX, FLT_MAX);
                controller.window.styleMask |= NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;

                CGDirectDisplayID display =
                    std::numeric_limits<gerium_uint32_t>::max() == displayId ? kCGDirectMainDisplay : displayId;

                if (mode) {
                    CFArrayRef modes = CGDisplayCopyAllDisplayModes(display, nil);
                    long modeCount   = CFArrayGetCount(modes);

                    for (long m = 0; m < modeCount; ++m) {
                        CGDisplayModeRef ref = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, m);
                        size_t width         = CGDisplayModeGetWidth(ref);
                        size_t height        = CGDisplayModeGetHeight(ref);
                        double refreshRate   = CGDisplayModeGetRefreshRate(ref);

                        if (width == mode->width && height == mode->height && int(refreshRate) == mode->refresh_rate) {
                            // CGDisplayCapture(kCGDirectMainDisplay);
                            CGDisplayConfigRef config;
                            CGBeginDisplayConfiguration(&config);
                            CGConfigureDisplayWithDisplayMode(config, display, ref, nil);
                            CGCompleteDisplayConfiguration(config, kCGConfigureForAppOnly);
                            break;
                        }
                    }
                }
            }
            [controller.window toggleFullScreen:controller];
        }
    } else {
        _startFullscreen = fullscreen;
        _display         = displayId;
        if (mode) {
            _mode = *mode;
        }
    }
}

gerium_application_style_flags_t MacOSApplication::onGetStyle() const noexcept {
    return _styles;
}

void MacOSApplication::onSetStyle(gerium_application_style_flags_t style) noexcept {
    _styles = style;

    if (!isFullscreen()) {
        WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
        NSWindowStyleMask mask           = controller.window.styleMask;

        if (style & GERIUM_APPLICATION_STYLE_RESIZABLE_BIT) {
            mask |= NSWindowStyleMaskResizable;
        } else {
            mask ^= NSWindowStyleMaskResizable;
        }

        if (style & GERIUM_APPLICATION_STYLE_MINIMIZABLE_BIT) {
            mask |= NSWindowStyleMaskMiniaturizable;
        } else {
            mask ^= NSWindowStyleMaskMiniaturizable;
        }

        controller.window.styleMask = mask;
    }
}

void MacOSApplication::onGetMinSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
    if (width) {
        *width = getPixelSize(controller.window.minSize.width);
    }
    if (height) {
        *height = getPixelSize(controller.window.minSize.height - titlebarHeight());
    }
}

void MacOSApplication::onGetMaxSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
    if (width) {
        *width = getPixelSize(controller.window.maxSize.width);
    }
    if (height) {
        *height = getPixelSize(controller.window.maxSize.height - titlebarHeight());
    }
}

void MacOSApplication::onGetSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
    NSRect frame                     = [controller.window frame];
    if (width) {
        *width = getPixelSize(frame.size.width);
    }
    if (height) {
        *height = getPixelSize(frame.size.height - titlebarHeight());
    }
}

void MacOSApplication::onSetMinSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    if (!isFullscreen()) {
        WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
        controller.window.minSize        = NSMakeSize(getDeviceSize(width), getDeviceSize(height) + titlebarHeight());

        gerium_uint16_t currentWidth;
        gerium_uint16_t currentHeight;
        onGetSize(&currentWidth, &currentHeight);
        if (currentWidth < width || currentHeight < height) {
            width  = currentWidth < width ? width : currentWidth;
            height = currentHeight < height ? height : currentHeight;
            onSetSize(width, height);
        }
    }
    _newMinWidth  = width;
    _newMinHeight = height;
}

void MacOSApplication::onSetMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    if (!isFullscreen()) {
        WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
        controller.window.maxSize        = NSMakeSize(getDeviceSize(width), getDeviceSize(height) + titlebarHeight());

        gerium_uint16_t currentWidth;
        gerium_uint16_t currentHeight;
        onGetSize(&currentWidth, &currentHeight);
        if (currentWidth > width || currentHeight > height) {
            width  = currentWidth > width ? width : currentWidth;
            height = currentHeight > height ? height : currentHeight;
            onSetSize(width, height);
        }
    }
    _newMaxWidth  = width;
    _newMaxHeight = height;
}

void MacOSApplication::onSetSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    if (!isFullscreen()) {
        WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
        NSRect frame                     = [controller.window frame];
        frame.size.width                 = getDeviceSize(width);
        frame.size.height                = getDeviceSize(height) + titlebarHeight();
        [controller.window setFrame:frame display:YES animate:YES];
    }
    _newWidth  = width;
    _newHeight = height;
}

gerium_utf8_t MacOSApplication::onGetTitle() const noexcept {
    WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
    return [controller.window.title UTF8String];
}

void MacOSApplication::onSetTitle(gerium_utf8_t title) noexcept {
    WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
    controller.window.title          = [NSString stringWithUTF8String:title];
}

void MacOSApplication::onShowCursor(bool show) noexcept {
    if (show) {
        [NSCursor unhide];
    } else {
        [NSCursor hide];
    }
}

void MacOSApplication::onRun() {
    if (_exited) {
        error(GERIUM_RESULT_ERROR_APPLICATION_TERMINATED);
    }
    if (_running) {
        error(GERIUM_RESULT_ERROR_APPLICATION_ALREADY_RUNNING);
    }
    @try {
        _running                   = true;
        _prevTime                  = getCurrentTime();
        NSApplication* application = [NSApplication sharedApplication];
        [application setDelegate:((__bridge WindowViewController*) _viewController)];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
    } @catch (NSException*) {
        error(GERIUM_RESULT_ERROR_UNKNOWN);
    }
}

void MacOSApplication::onExit() noexcept {
    if (_running) {
        WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
        [controller.window close];
    }
    _exited = true;
}

bool MacOSApplication::onIsRunning() const noexcept {
    return _running;
}

void MacOSApplication::onInitImGui() {
    MTKView* view                    = ((__bridge MTKView*) _view);
    WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
    ImGui_ImplOSX_Init(view);
    [controller.window makeFirstResponder:controller];
}

void MacOSApplication::onShutdownImGui() {
    ImGui_ImplOSX_Shutdown();
}

void MacOSApplication::onNewFrameImGui() {
    MTKView* view = ((__bridge MTKView*) _view);
    ImGui_ImplOSX_NewFrame(view);
}

void MacOSApplication::enumDisplays(const std::vector<CGDirectDisplayID>& activeDisplays,
                                    gerium_uint32_t displayCount,
                                    bool isMain,
                                    gerium_uint32_t& index,
                                    gerium_display_info_t* displays) const {
    for (uint32_t i = 0; i < displayCount; ++i) {
        if (CGDisplayIsMain(activeDisplays[i]) != isMain) {
            continue;
        }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        NSDictionary* deviceInfo = CFBridgingRelease(
            IODisplayCreateInfoDictionary(CGDisplayIOServicePort(activeDisplays[i]), kIODisplayOnlyPreferredName));
#pragma clang diagnostic pop

        NSDictionary* localizedNames = deviceInfo[@(kDisplayProductName)];

        if (localizedNames.count > 0) {
            _displayNames.push_back([localizedNames.allValues[0] UTF8String]);
        } else {
            _displayNames.push_back("Unknown");
        }

        _displayNames.push_back("Unknown");
        _displayNames.push_back(std::to_string(CGDisplayUnitNumber(activeDisplays[i])));

        CFArrayRef modes = CGDisplayCopyAllDisplayModes(activeDisplays[i], nil);
        long modeCount   = CFArrayGetCount(modes);
        _modes.resize(modeCount);
        for (long m = 0; m < modeCount; ++m) {
            CGDisplayModeRef mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, m);
            size_t width          = CGDisplayModeGetWidth(mode);
            size_t height         = CGDisplayModeGetHeight(mode);
            double refreshRate    = CGDisplayModeGetRefreshRate(mode);

            auto& result  = _modes[m];
            result.width  = gerium_uint16_t(width);
            result.height = gerium_uint16_t(height);
        }
        displays[index].id         = activeDisplays[i];
        displays[index].mode_count = gerium_uint32_t(modeCount);
        ++index;

        if (CGDisplayIsMain(activeDisplays[i]) == isMain) {
            break;
        }
    }
}

std::chrono::high_resolution_clock::time_point MacOSApplication::getCurrentTime() const noexcept {
    return std::chrono::high_resolution_clock::now();
}

} // namespace gerium::macos

gerium_result_t gerium_application_create(gerium_utf8_t title,
                                          gerium_uint32_t width,
                                          gerium_uint32_t height,
                                          gerium_application_t* application) {
    using namespace gerium;
    using namespace gerium::macos;
    return Object::create<MacOSApplication>(*application, title, width, height);
}
