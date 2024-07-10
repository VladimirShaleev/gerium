#ifndef GERIUM_ANDROID_ANDROID_APPLICATION_HPP
#define GERIUM_ANDROID_ANDROID_APPLICATION_HPP

#include "../Application.hpp"

#include <android/native_window.h>
#include <android/configuration.h>
#include <android_native_app_glue.h>

namespace gerium::android {

class AndroidApplication final : public Application {
public:
    AndroidApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height);

    ANativeWindow* nativeWindow() noexcept;

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

    void onRun() override;
    void onExit() noexcept override;

    bool onIsRunning() const noexcept override;

    void onInitImGui() override;
    void onShutdownImGui() override;
    void onNewFrameImGui() override;

    void initialize();
    void uninitialize();
    void active();
    void deactive();
    void gotFocus();
    void lostFocus();
    bool isPause() const noexcept;
    void onAppCmd(int32_t cmd) noexcept;

    static void onAppCmd(android_app* application, int32_t cmd);
    static std::chrono::high_resolution_clock::time_point getCurrentTime() noexcept;

    android_app* _application;
    bool _initialized;
    bool _activated;
    bool _focused;
    bool _exit;
    bool _callbackError;
    jmethodID _isInMultiWindowMode;
    std::chrono::high_resolution_clock::time_point _prevTime;
};

} // namespace gerium::android

#endif
