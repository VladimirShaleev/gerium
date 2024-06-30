#include "MacOSApplication.hpp"

#import <MetalKit/MetalKit.h>

@interface WindowViewController : NSViewController <NSApplicationDelegate, NSWindowDelegate, MTKViewDelegate> {
    @public gerium::macos::MacOSApplication* application;
}

@property (strong, nonatomic) NSWindow *window;

@end

@implementation WindowViewController

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    if (!application->changeState(GERIUM_APPLICATION_STATE_RESIZE)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
}

- (void)drawInMTKView:(nonnull MTKView *)view {
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    if (!application->changeState(GERIUM_APPLICATION_STATE_CREATE)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
    if (!application->changeState(GERIUM_APPLICATION_STATE_INITIALIZE)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
    if (application->isStartedFullscreen()) {
        application->fullscreen(true);
    }
    if (!application->isFullscreen()) {
        if (!application->changeState(GERIUM_APPLICATION_STATE_NORMAL)) {
            application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
        }
    }
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    if ([NSApp occlusionState] & NSApplicationOcclusionStateVisible) {
        if (!application->changeState(GERIUM_APPLICATION_STATE_INVISIBLE)) {
            application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
        }
    }
    if (!application->changeState(GERIUM_APPLICATION_STATE_UNINITIALIZE)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
    if (!application->changeState(GERIUM_APPLICATION_STATE_DESTROY)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    if (!application->changeState(GERIUM_APPLICATION_STATE_GOT_FOCUS)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
}

- (void)applicationDidResignActive:(NSNotification *)notification {
    if (!application->changeState(GERIUM_APPLICATION_STATE_LOST_FOCUS)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
}

- (void)applicationDidChangeOcclusionState:(NSNotification *)notification {
    if ([NSApp occlusionState] & NSApplicationOcclusionStateVisible) {
        if (!application->changeState(GERIUM_APPLICATION_STATE_VISIBLE)) {
            application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
        }
    } else {
        if (!application->changeState(GERIUM_APPLICATION_STATE_INVISIBLE)) {
            application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
        }
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (void)windowDidEndLiveResize:(NSNotification *)notification {
    if (!application->changeState(GERIUM_APPLICATION_STATE_RESIZED)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
}

- (void)windowDidMiniaturize:(NSNotification *)notification {
    if (!application->changeState(GERIUM_APPLICATION_STATE_MINIMIZE)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
    if (!application->changeState(GERIUM_APPLICATION_STATE_INVISIBLE)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
}

- (void)windowDidDeminiaturize:(NSNotification *)notification {
    if (!application->changeState(GERIUM_APPLICATION_STATE_NORMAL)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
    if ([NSApp occlusionState] & NSApplicationOcclusionStateVisible) {
        if (!application->changeState(GERIUM_APPLICATION_STATE_VISIBLE)) {
            application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
        }
    }
}

- (void)windowDidEnterFullScreen:(NSNotification *)notification {
    if (!application->changeState(GERIUM_APPLICATION_STATE_FULLSCREEN)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
}

- (void)windowDidExitFullScreen:(NSNotification *)notification {
    if (!application->changeState(GERIUM_APPLICATION_STATE_NORMAL)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
    application->setSize();
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
        viewController->application = this;
        
        MTKView* view = [[MTKView alloc] initWithFrame:frame];
        view.delegate = viewController;
        view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        
        NSWindow* window = [[NSWindow alloc]
                            initWithContentRect:frame
                            styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable)
                            backing:NSBackingStoreBuffered
                            defer:NO
        ];
        window.delegate = viewController;
        window.title = [NSString stringWithUTF8String:title];
        
        [window.contentView addSubview:view];
        [window center];
        [window orderFrontRegardless];
        
        viewController.window = window;
        _scale = window.backingScaleFactor;
        _invScale = 1.0f / _scale;
        _viewController = CFRetain((__bridge void*) viewController);
        _view = CFRetain((__bridge void*) view);
        
    } @catch (NSException*) {
        error(GERIUM_RESULT_ERROR_UNKNOWN);
    }
}

bool MacOSApplication::changeState(gerium_application_state_t newState) {
    if (newState != _prevState || newState == GERIUM_APPLICATION_STATE_RESIZE) {
        _prevState = newState;
        return callStateFunc(newState);
    }
    return true;
}

bool MacOSApplication::isStartedFullscreen() const noexcept {
    return _startFullscreen;
}

bool MacOSApplication::isFullscreen() const noexcept {
    return onIsFullscreen();
}
void MacOSApplication::setSize() noexcept {
    constexpr auto val = std::numeric_limits<gerium_uint16_t>::max();
    
    if (_newMinWidth != val) {
        onSetMinSize(_newMinWidth, _newMinHeight);
        _newMinWidth = val;
        _newMinHeight = val;
    }
    if (_newMaxWidth != val) {
        onSetMaxSize(_newMaxWidth, _newMaxHeight);
        _newMaxWidth = val;
        _newMaxHeight = val;
    }
    if (_newWidth != val) {
        onSetSize(_newWidth, _newHeight);
        _newWidth = val;
        _newHeight = val;
    }
}

void MacOSApplication::fullscreen(bool fullscreen) noexcept {
    onFullscreen(fullscreen, nullptr);
}
gerium_uint16_t MacOSApplication::getPixelSize(gerium_uint16_t x) const noexcept {
    return gerium_uint16_t(x * _scale);
}

float MacOSApplication::getDeviceSize(gerium_uint16_t x) const noexcept {
    return x * _invScale;
}

float MacOSApplication::titlebarHeight() const noexcept {
    WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
    NSRect frame = controller.window.frame;
    return frame.size.height - [controller.window contentRectForFrameRect:frame].size.height;
}

void MacOSApplication::error(gerium_result_t result) const {
    Application::error(result);
}

gerium_runtime_platform_t MacOSApplication::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_MAC_OS;
}

void MacOSApplication::onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const {
    displayCount = 0;
}

bool MacOSApplication::onIsFullscreen() const noexcept {
    if (_running) {
        WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
        return [controller.window styleMask] & NSWindowStyleMaskFullScreen;
    } else {
        return _startFullscreen;
    }
}

void MacOSApplication::onFullscreen(bool fullscreen, const gerium_display_mode_t* mode) {
    if (_running) {
        if (fullscreen != onIsFullscreen()) {
            WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
            if (fullscreen) {
                controller.window.minSize = NSMakeSize(0, 0);
                controller.window.maxSize = NSMakeSize(FLT_MAX, FLT_MAX);
            }
            [controller.window toggleFullScreen:controller];
        }
    } else {
        _startFullscreen = fullscreen;
    }
}

gerium_application_style_flags_t MacOSApplication::onGetStyle() const noexcept {
    return {};
}

void MacOSApplication::onSetStyle(gerium_application_style_flags_t style) noexcept {
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
    NSRect frame = [controller.window frame];
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
        controller.window.minSize = NSMakeSize(getDeviceSize(width), getDeviceSize(height) + titlebarHeight());
        
        gerium_uint16_t currentWidth;
        gerium_uint16_t currentHeight;
        onGetSize(&currentWidth, &currentHeight);
        if (currentWidth < width || currentHeight < height) {
            width = currentWidth < width ? width : currentWidth;
            height = currentHeight < height ? height : currentHeight;
            onSetSize(width, height);
        }
    }
    _newMinWidth = width;
    _newMinHeight = height;
}

void MacOSApplication::onSetMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    if (!isFullscreen()) {
        WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
        controller.window.maxSize = NSMakeSize(getDeviceSize(width), getDeviceSize(height) + titlebarHeight());
        
        gerium_uint16_t currentWidth;
        gerium_uint16_t currentHeight;
        onGetSize(&currentWidth, &currentHeight);
        if (currentWidth > width || currentHeight > height) {
            width = currentWidth > width ? width : currentWidth;
            height = currentHeight > height ? height : currentHeight;
            onSetSize(width, height);
        }
    }
    _newMaxWidth = width;
    _newMaxHeight = height;
}

void MacOSApplication::onSetSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    if (!isFullscreen()) {
        WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
        NSRect frame = [controller.window frame];
        frame.size.width = getDeviceSize(width);
        frame.size.height = getDeviceSize(height) + titlebarHeight();
        [controller.window setFrame:frame display:YES animate:YES];
    }
    _newWidth = width;
    _newHeight = height;
}

gerium_utf8_t MacOSApplication::onGetTitle() const noexcept {
    WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
    return [controller.window.title UTF8String];
}

void MacOSApplication::onSetTitle(gerium_utf8_t title) noexcept {
    WindowViewController* controller = ((__bridge WindowViewController*) _viewController);
    controller.window.title = [NSString stringWithUTF8String:title];
}

void MacOSApplication::onRun() {
    if (_exited) {
        error(GERIUM_RESULT_ERROR_APPLICATION_TERMINATED);
    }
    if (_running) {
        error(GERIUM_RESULT_ERROR_APPLICATION_ALREADY_RUNNING);
    }
    @try {
        _running = true;
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

} // namespace gerium::macos

gerium_result_t gerium_application_create(gerium_utf8_t title,
                                          gerium_uint32_t width,
                                          gerium_uint32_t height,
                                          gerium_application_t* application) {
    using namespace gerium;
    using namespace gerium::macos;
    return Object::create<MacOSApplication>(*application, title, width, height);
}
