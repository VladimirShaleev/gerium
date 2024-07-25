#ifndef GERIUM_ANDROID_ANDROID_APPLICATION_HPP
#define GERIUM_ANDROID_ANDROID_APPLICATION_HPP

#include "../Application.hpp"

#include <android_native_app_glue.h>

namespace gerium::android {

class AndroidApplication final : public Application {
public:
    AndroidApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height);

    ANativeWindow* nativeWindow() noexcept;

    static android_app* instance() noexcept;

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
    int getUnicodeChar(int eventType, int keyCode, int metaState) const;
    bool showSoftKeyboard(bool show);
    void onAppCmd(int32_t cmd) noexcept;
    int32_t onInputEvent(AInputEvent* event) noexcept;

    static void onAppCmd(android_app* application, int32_t cmd);
    static int32_t onInputEvent(android_app* app, AInputEvent* event);
    static ImGuiKey keyCodeToImGuiKey(int32_t keyCode) noexcept;

    static std::chrono::high_resolution_clock::time_point getCurrentTime() noexcept;

    android_app* _application;
    bool _initialized;
    bool _activated;
    bool _focused;
    bool _exit;
    mutable std::string _title;
    mutable std::vector<std::string> _names;
    mutable std::vector<gerium_display_mode_t> _modes;
    jmethodID _isInMultiWindowMode;
    std::chrono::high_resolution_clock::time_point _prevTime;

    jclass _keyEventClass;
    jmethodID _keyEventCtor;
    jmethodID _getUnicodeCharMethod;
    jmethodID _getUnicodeCharIMethod;

    jobject _inputMethodManager;
    jmethodID _getWindowMethod;
    jmethodID _getDecorViewMethod;
    jmethodID _getWindowTokenMethod;
    jmethodID _showSoftInputMethod;
    jmethodID _hideSoftInputFromWindowMehtod;

    jobject _displayManager;
    jmethodID _getDisplays;
    jmethodID _getDisplayName;
    jmethodID _getDisplayId;
    jmethodID _getSupportedModes;
    jmethodID _getPhysicalHeight;
    jmethodID _getPhysicalWidth;
    jmethodID _getRefreshRate;

    jmethodID _getTitle;
    jmethodID _setTitle;
};

} // namespace gerium::android

#endif
