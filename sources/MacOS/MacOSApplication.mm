#include "MacOSApplication.hpp"

#import <MetalKit/MetalKit.h>

@interface WindowViewController : NSViewController <NSApplicationDelegate, NSWindowDelegate, MTKViewDelegate> {
    @public gerium::macos::MacOSApplication* application;
}

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
}

- (void)applicationWillTerminate:(NSNotification *)notification {
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

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (void)windowDidEndLiveResize:(NSNotification *)notification {
    if (!application->changeState(GERIUM_APPLICATION_STATE_RESIZED)) {
        application->error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
    }
}

@end

namespace gerium::macos {

MacOSApplication::MacOSApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) {
    @try {
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
    return false;
}

void MacOSApplication::onFullscreen(bool fullscreen, const gerium_display_mode_t* mode) {
}

gerium_application_style_flags_t MacOSApplication::onGetStyle() const noexcept {
    return {};
}

void MacOSApplication::onSetStyle(gerium_application_style_flags_t style) noexcept {
}

void MacOSApplication::onGetMinSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
}

void MacOSApplication::onGetMaxSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
}

void MacOSApplication::onGetSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
}

void MacOSApplication::onSetMinSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
}

void MacOSApplication::onSetMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
}

void MacOSApplication::onSetSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
}

gerium_utf8_t MacOSApplication::onGetTitle() const noexcept {
    return "";
}

void MacOSApplication::onSetTitle(gerium_utf8_t title) noexcept {
}

void MacOSApplication::onRun() {
    @try {
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
