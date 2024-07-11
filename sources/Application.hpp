#ifndef GERIUM_APPLICATION_HPP
#define GERIUM_APPLICATION_HPP

#include "ObjectPtr.hpp"

struct _gerium_application : public gerium::Object {};

namespace gerium {

class Application : public _gerium_application {
public:
    Application() noexcept;

    gerium_runtime_platform_t getPlatform() const noexcept;

    gerium_application_frame_func_t getFrameFunc(gerium_data_t* data) const noexcept;
    gerium_application_state_func_t getStateFunc(gerium_data_t* data) const noexcept;

    void setFrameFunc(gerium_application_frame_func_t callback, gerium_data_t data) noexcept;
    void setStateFunc(gerium_application_state_func_t callback, gerium_data_t data) noexcept;

    gerium_result_t getDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const noexcept;

    bool isFullscreen() const noexcept;
    gerium_result_t fullscreen(bool fullscreen, gerium_uint32_t displayId, const gerium_display_mode_t* mode) noexcept;

    gerium_application_style_flags_t getStyle() const noexcept;
    void setStyle(gerium_application_style_flags_t style) noexcept;

    void getMinSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept;
    void getMaxSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept;
    void getSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept;
    void setMinSize(gerium_uint16_t width, gerium_uint16_t height) noexcept;
    void setMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept;
    void setSize(gerium_uint16_t width, gerium_uint16_t height) noexcept;

    gerium_utf8_t getTitle() const noexcept;
    void setTitle(gerium_utf8_t title) noexcept;

    bool getBackgroundWait() const noexcept;
    void setBackgroundWait(bool enable) noexcept;

    gerium_result_t run() noexcept;
    void exit() noexcept;

    bool isRunning() const noexcept;

    void initImGui();
    void shutdownImGui();
    void newFrameImGui();

protected:
    gerium_application_state_t currentState() const noexcept;
    gerium_bool_t callbackStateFailed() const noexcept;
    void changeState(gerium_application_state_t newState, bool noThrow = false);
    bool callFrameFunc(gerium_float32_t elapsed) noexcept;
    bool callStateFunc(gerium_application_state_t state) noexcept;

private:
    virtual gerium_runtime_platform_t onGetPlatform() const noexcept = 0;

    virtual void onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const = 0;

    virtual bool onIsFullscreen() const noexcept                                                             = 0;
    virtual void onFullscreen(bool fullscreen, gerium_uint32_t displayId, const gerium_display_mode_t* mode) = 0;

    virtual gerium_application_style_flags_t onGetStyle() const noexcept     = 0;
    virtual void onSetStyle(gerium_application_style_flags_t style) noexcept = 0;

    virtual void onGetMinSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept = 0;
    virtual void onGetMaxSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept = 0;
    virtual void onGetSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept    = 0;
    virtual void onSetMinSize(gerium_uint16_t width, gerium_uint16_t height) noexcept         = 0;
    virtual void onSetMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept         = 0;
    virtual void onSetSize(gerium_uint16_t width, gerium_uint16_t height) noexcept            = 0;

    virtual gerium_utf8_t onGetTitle() const noexcept     = 0;
    virtual void onSetTitle(gerium_utf8_t title) noexcept = 0;

    virtual void onRun()           = 0;
    virtual void onExit() noexcept = 0;

    virtual bool onIsRunning() const noexcept = 0;

    virtual void onInitImGui()     = 0;
    virtual void onShutdownImGui() = 0;
    virtual void onNewFrameImGui() = 0;

    gerium_application_frame_func_t _frameFunc;
    gerium_application_state_func_t _stateFunc;
    gerium_data_t _frameData;
    gerium_data_t _stateData;
    gerium_bool_t _backgroundWait;
    gerium_application_state_t _currentState;
    gerium_bool_t _callbackStateFailed;
};

} // namespace gerium

#endif
