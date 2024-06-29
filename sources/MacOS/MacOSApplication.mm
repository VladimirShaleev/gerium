#include "MacOSApplication.hpp"

namespace gerium::macos {

MacOSApplication::MacOSApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) {

}

gerium_runtime_platform_t MacOSApplication::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_MAC_OS;
}

void MacOSApplication::onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const {
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
