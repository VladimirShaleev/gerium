#ifndef GERIUM_MAC_OS_MAC_OS_APPLICATION_HPP
#define GERIUM_MAC_OS_MAC_OS_APPLICATION_HPP

#include "../Application.hpp"

#import <Cocoa/Cocoa.h>

namespace gerium::macos {

class MacOSApplication final : public Application {
public:
    MacOSApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height);

    ~MacOSApplication() {
        if (_view) {
            CFRelease(_view);
        }
        if (_viewController) {
            CFRelease(_viewController);
        }
    }
    
    bool changeState(gerium_application_state_t newState);
    
    bool isStartedFullscreen() const noexcept;
    bool isFullscreen() const noexcept;
    
    void fullscreen(bool fullscreen) noexcept;
    
    [[noreturn]] void error(gerium_result_t result) const;
    
private:
    gerium_runtime_platform_t onGetPlatform() const noexcept override;

    void onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const override;

    bool onIsFullscreen() const noexcept override;
    void onFullscreen(bool fullscreen, const gerium_display_mode_t* mode) override;

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

    void onRun() override;
    void onExit() noexcept override;
    
    const void* _viewController = nullptr;
    const void* _view = nullptr;
    bool _running = false;
    bool _exited = false;
    bool _startFullscreen = false;
    gerium_application_state_t _prevState = GERIUM_APPLICATION_STATE_UNKNOWN;
};

} // namespace gerium::macos

#endif
