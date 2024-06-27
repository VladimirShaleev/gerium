#ifndef GERIUM_APPLICATION_HPP
#define GERIUM_APPLICATION_HPP

#include "Object.hpp"

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

    gerium_result_t run() noexcept;
    void exit() noexcept;

protected:
    bool callFrameFunc(gerium_float32_t elapsed) noexcept;
    bool callStateFunc(gerium_application_state_t state) noexcept;

private:
    virtual gerium_runtime_platform_t onGetPlatform() const noexcept = 0;

    virtual void onRun()           = 0;
    virtual void onExit() noexcept = 0;

    gerium_application_frame_func_t _frameFunc;
    gerium_application_state_func_t _stateFunc;
    gerium_data_t _frameData;
    gerium_data_t _stateData;
};

} // namespace gerium

#endif