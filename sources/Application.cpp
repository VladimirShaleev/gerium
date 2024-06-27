#include "Application.hpp"

namespace gerium {

gerium_runtime_platform_t Application::getPlatform() const noexcept {
    return onGetPlatform();
}

gerium_result_t Application::run() noexcept {
    return invoke<Application>([](auto obj) {
        obj->onRun();
    });
}

void Application::exit() noexcept {
    onExit();
}

} // namespace gerium

using namespace gerium;

gerium_application_t gerium_application_reference(gerium_application_t application) {
    assert(application);
    application->reference();
    return application;
}

void gerium_application_destroy(gerium_application_t application) {
    if (application) {
        application->destroy();
    }
}

gerium_runtime_platform_t gerium_application_get_platform(gerium_application_t application) {
    assert(application);
    return alias_cast<Application*>(application)->getPlatform();
}

gerium_result_t gerium_application_run(gerium_application_t application) {
    assert(application);
    return alias_cast<Application*>(application)->run();
}

void gerium_application_exit(gerium_application_t application) {
    assert(application);
    alias_cast<Application*>(application)->exit();
}
