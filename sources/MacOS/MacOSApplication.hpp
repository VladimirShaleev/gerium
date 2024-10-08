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

    const CAMetalLayer* layer() const noexcept;

    void changeState(gerium_application_state_t newState);
    void frame();

    bool isStartedFullscreen() const noexcept;
    bool isFullscreen() const noexcept;
    bool isHideCursor() const noexcept;

    void restoreWindow() noexcept;
    void fullscreen(bool fullscreen) noexcept;

    gerium_uint16_t getPixelSize(gerium_uint16_t x) const noexcept;
    float getDeviceSize(gerium_uint16_t x) const noexcept;

    float titlebarHeight() const noexcept;

    const void* getView() const noexcept;

    bool isPressed(gerium_scancode_t scancode) const noexcept;
    void setPressed(gerium_scancode_t scancode, bool pressed) noexcept;
    void sendEvent(const gerium_event_t& event) noexcept;
    void clearEvents() noexcept;

    float scale() const noexcept;

    static gerium_uint64_t ticks() noexcept;

private:
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

    void enumDisplays(const std::vector<CGDirectDisplayID>& activeDisplays,
                      gerium_uint32_t displayCount,
                      bool isMain,
                      gerium_uint32_t& index,
                      gerium_display_info_t* displays) const;

    std::chrono::high_resolution_clock::time_point getCurrentTime() const noexcept;

    const void* _viewController                = nullptr;
    const void* _view                          = nullptr;
    bool _running                              = false;
    bool _exited                               = false;
    bool _startFullscreen                      = false;
    gerium_uint32_t _display                   = std::numeric_limits<gerium_uint32_t>::max();
    std::optional<gerium_display_mode_t> _mode = {};
    float _scale                               = 1.0f;
    float _invScale                            = 1.0f;
    gerium_uint16_t _newMinWidth               = std::numeric_limits<gerium_uint16_t>::max();
    gerium_uint16_t _newMinHeight              = std::numeric_limits<gerium_uint16_t>::max();
    gerium_uint16_t _newMaxWidth               = std::numeric_limits<gerium_uint16_t>::max();
    gerium_uint16_t _newMaxHeight              = std::numeric_limits<gerium_uint16_t>::max();
    gerium_uint16_t _newWidth                  = std::numeric_limits<gerium_uint16_t>::max();
    gerium_uint16_t _newHeight                 = std::numeric_limits<gerium_uint16_t>::max();
    gerium_application_style_flags_t _styles   = GERIUM_APPLICATION_STYLE_RESIZABLE_BIT |
                                               GERIUM_APPLICATION_STYLE_MINIMIZABLE_BIT |
                                               GERIUM_APPLICATION_STYLE_MAXIMIZABLE_BIT;
    mutable std::vector<gerium_display_mode_t> _modes        = {};
    mutable std::vector<std::string> _displayNames           = {};
    std::chrono::high_resolution_clock::time_point _prevTime = {};
};

} // namespace gerium::macos

#endif
