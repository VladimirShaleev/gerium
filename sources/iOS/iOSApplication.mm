#include "iOSApplication.hpp"

namespace gerium::ios {

iOSApplication::iOSApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) {
}

gerium_runtime_platform_t iOSApplication::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_IOS;
}

void iOSApplication::onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const {

}

bool iOSApplication::onIsFullscreen() const noexcept {
    return {};
}

void iOSApplication::onFullscreen(bool fullscreen, gerium_uint32_t displayId, const gerium_display_mode_t* mode) {

}

gerium_application_style_flags_t iOSApplication::onGetStyle() const noexcept {
    return {};
}

void iOSApplication::onSetStyle(gerium_application_style_flags_t style) noexcept {

}

void iOSApplication::onGetMinSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {

}

void iOSApplication::onGetMaxSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {

}

void iOSApplication::onGetSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {

}

void iOSApplication::onSetMinSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {

}

void iOSApplication::onSetMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {

}

void iOSApplication::onSetSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {

}
gerium_utf8_t iOSApplication::onGetTitle() const noexcept {
    return "";
}

void iOSApplication::onSetTitle(gerium_utf8_t title) noexcept {

}

void iOSApplication::onShowCursor(bool show) noexcept {

}

void iOSApplication::onRun() {

}

void iOSApplication::onExit() noexcept {

}

bool iOSApplication::onIsRunning() const noexcept {
    return {};
}

void iOSApplication::onInitImGui() {

}

void iOSApplication::onShutdownImGui() {

}

void iOSApplication::onNewFrameImGui() {

}

} // namespace gerium::ios

gerium_result_t gerium_application_create(gerium_utf8_t title,
                                          gerium_uint32_t width,
                                          gerium_uint32_t height,
                                          gerium_application_t* application) {
    using namespace gerium;
    using namespace gerium::ios;
    return Object::create<iOSApplication>(*application, title, width, height);
}
