#ifndef GERIUM_APPLICATION_HPP
#define GERIUM_APPLICATION_HPP

#include "Object.hpp"

struct _gerium_application : public gerium::Object {};

namespace gerium {

class Application : public _gerium_application {
public:
    gerium_runtime_platform_t getPlatform() const noexcept;

    void run();
    void exit() noexcept;

protected:
private:
    virtual gerium_runtime_platform_t onGetPlatform() const noexcept = 0;
    virtual void onRun() = 0;
    virtual void onExit() noexcept = 0;
};

} // namespace gerium

#endif
