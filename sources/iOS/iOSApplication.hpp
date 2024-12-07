#ifndef GERIUM_IOS_IOS_APPLICATION_HPP
#define GERIUM_IOS_IOS_APPLICATION_HPP

#include "../Application.hpp"

namespace gerium::ios {

class iOSApplication final : public Application {
public:
    iOSApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height);
    
    
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
};

} // namespace gerium::ios

#endif
