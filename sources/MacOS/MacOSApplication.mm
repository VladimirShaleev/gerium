#include "MacOSApplication.hpp"

#include <imgui_impl_osx.h>

#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>
#import <IOKit/graphics/IOGraphicsLib.h>

@interface WindowViewController : NSViewController <NSApplicationDelegate, NSWindowDelegate, MTKViewDelegate> {
    @public gerium::macos::MacOSApplication* application;
}

@property (strong, nonatomic) NSWindow *window;

@end

@implementation WindowViewController

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    application->changeState(GERIUM_APPLICATION_STATE_RESIZE);
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    application->frame();
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    application->changeState(GERIUM_APPLICATION_STATE_CREATE);
    application->changeState(GERIUM_APPLICATION_STATE_INITIALIZE);
    if (application->isStartedFullscreen()) {
        application->fullscreen(true);
    }
    if (!application->isFullscreen()) {
        application->changeState(GERIUM_APPLICATION_STATE_NORMAL);
    }
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    if ([NSApp occlusionState] & NSApplicationOcclusionStateVisible) {
        application->changeState(GERIUM_APPLICATION_STATE_INVISIBLE);
    }
    application->changeState(GERIUM_APPLICATION_STATE_UNINITIALIZE);
    application->changeState(GERIUM_APPLICATION_STATE_DESTROY);
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    application->changeState(GERIUM_APPLICATION_STATE_GOT_FOCUS);
    
    MTKView* view = ((__bridge MTKView*) application->getView());
    view.paused = NO;
}

- (void)applicationDidResignActive:(NSNotification *)notification {
    application->changeState(GERIUM_APPLICATION_STATE_LOST_FOCUS);
    
    if (application->getBackgroundWait()) {
        MTKView* view = ((__bridge MTKView*) application->getView());
        view.paused = YES;
    }
}

- (void)applicationDidChangeOcclusionState:(NSNotification *)notification {
    if ([NSApp occlusionState] & NSApplicationOcclusionStateVisible) {
        application->changeState(GERIUM_APPLICATION_STATE_VISIBLE);
    } else {
        application->changeState(GERIUM_APPLICATION_STATE_INVISIBLE);
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (void)windowDidEndLiveResize:(NSNotification *)notification {
    application->changeState(GERIUM_APPLICATION_STATE_RESIZED);
}

- (void)windowDidMiniaturize:(NSNotification *)notification {
    application->changeState(GERIUM_APPLICATION_STATE_MINIMIZE);
    application->changeState(GERIUM_APPLICATION_STATE_INVISIBLE);
}

- (void)windowDidDeminiaturize:(NSNotification *)notification {
    application->changeState(GERIUM_APPLICATION_STATE_NORMAL);
    if ([NSApp occlusionState] & NSApplicationOcclusionStateVisible) {
        application->changeState(GERIUM_APPLICATION_STATE_VISIBLE);
    }
}

- (void)windowDidEnterFullScreen:(NSNotification *)notification {
    application->changeState(GERIUM_APPLICATION_STATE_FULLSCREEN);
}

- (void)windowDidExitFullScreen:(NSNotification *)notification {
    application->changeState(GERIUM_APPLICATION_STATE_NORMAL);
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
    const auto elapsed = delta.count();
    
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
void MacOSApplication::restoreWindow() noexcept {
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
    NSRect frame = controller.window.frame;
    return frame.size.height - [controller.window contentRectForFrameRect:frame].size.height;
}

const void* MacOSApplication::getView() const noexcept {
    return _view;
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
                
                CGDirectDisplayID display = std::numeric_limits<gerium_uint32_t>::max() == displayId ? kCGDirectMainDisplay : displayId;
                
                if (mode) {
                    CFArrayRef modes = CGDisplayCopyAllDisplayModes(display, nil);
                    long modeCount = CFArrayGetCount(modes);
                    
                    for (long m = 0; m < modeCount; ++m) {
                        CGDisplayModeRef ref = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, m);
                        size_t width = CGDisplayModeGetWidth(ref);
                        size_t height = CGDisplayModeGetHeight(ref);
                        double refreshRate = CGDisplayModeGetRefreshRate(ref);
                        
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
        _display = displayId;
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
        NSWindowStyleMask mask = controller.window.styleMask;
        
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
        _prevTime = getCurrentTime();
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
    MTKView* view = ((__bridge MTKView*) _view);
    ImGui_ImplOSX_Init(view);
}

void MacOSApplication::onShutdownImGui() {
    ImGui_ImplOSX_Shutdown();
}

void MacOSApplication::onNewFrameImGui() {
    MTKView* view = ((__bridge MTKView*) _view);
    ImGui_ImplOSX_NewFrame(view);
}

void MacOSApplication::enumDisplays(const std::vector<CGDirectDisplayID>& activeDisplays, gerium_uint32_t displayCount, bool isMain, gerium_uint32_t& index, gerium_display_info_t* displays) const {
    for (uint32_t i = 0; i < displayCount; ++i) {
        if (CGDisplayIsMain(activeDisplays[i]) != isMain) {
            continue;
        }
        
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
        NSDictionary *deviceInfo = CFBridgingRelease(IODisplayCreateInfoDictionary(CGDisplayIOServicePort(activeDisplays[i]), kIODisplayOnlyPreferredName));
    #pragma clang diagnostic pop
        
        NSDictionary *localizedNames = deviceInfo[@(kDisplayProductName)];
          
        if (localizedNames.count > 0) {
            _displayNames.push_back([localizedNames.allValues[0] UTF8String]);
        } else {
            _displayNames.push_back("Unknown");
        }
        
        _displayNames.push_back("Unknown");
        _displayNames.push_back(std::to_string(CGDisplayUnitNumber(activeDisplays[i])));

        CFArrayRef modes = CGDisplayCopyAllDisplayModes(activeDisplays[i], nil);
        long modeCount = CFArrayGetCount(modes);
        _modes.resize(modeCount);
        for (long m = 0; m < modeCount; ++m) {
            CGDisplayModeRef mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, m);
            size_t width = CGDisplayModeGetWidth(mode);
            size_t height = CGDisplayModeGetHeight(mode);
            double refreshRate = CGDisplayModeGetRefreshRate(mode);
            
            auto& result = _modes[m];
            result.width = gerium_uint16_t(width);
            result.height = gerium_uint16_t(height);
        }
        displays[index].id = activeDisplays[i];
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
