#include "ServiceManager.hpp"
#include "../Application.hpp"

ServiceManager& Service::manager() noexcept {
    return *_manager;
}

const ServiceManager& Service::manager() const noexcept {
    return *_manager;
}

Application& Service::application() noexcept {
    return _manager->application();
}

const Application& Service::application() const noexcept {
    return _manager->application();
}

entt::registry& Service::entityRegistry() noexcept {
    return application().entityRegistry();
}

const entt::registry& Service::entityRegistry() const noexcept {
    return application().entityRegistry();
}

bool Service::isStopped() const noexcept {
    return _started == 0;
}

void Service::start() {
}

void Service::stop() {
}

void Service::setManager(ServiceManager* manager, uint32_t id) noexcept {
    _manager = manager;
    _id      = id;
}

void Service::setStopped(bool stop) {
    _started += stop ? -1 : 1;
    assert(_started >= 0);
    if (_started == 0) {
        this->stop();
    } else if (_started == 1) {
        start();
    }
}

uint32_t Service::serviceId() const noexcept {
    return _id;
}

void ServiceManager::create(Application* application) {
    assert(application);
    _application = application;
}

void ServiceManager::destroy() noexcept {
    for (auto it = _services.rbegin(); it != _services.rend(); ++it) {
        auto service = it->get();
        service->setStopped(true);
    }
    _hashServices.clear();
    _services.clear();
}

void ServiceManager::update(gerium_uint64_t elapsedMs) {
    _absoluteMs += elapsedMs;
    _elapsedMs = elapsedMs;
    _absolute  = _absoluteMs * 0.001;
    _elapsed   = _elapsedMs * 0.001;

    for (auto it = _services.begin(); it != _services.end(); ++it) {
        auto service = it->get();
        if (!service->isStopped()) {
            service->update(_elapsedMs, _elapsed);
        }
    }
}

Application& ServiceManager::application() noexcept {
    return *_application;
}

const Application& ServiceManager::application() const noexcept {
    return *_application;
}

gerium_uint64_t ServiceManager::elapsedMs() const noexcept {
    return _elapsedMs;
}

gerium_uint64_t ServiceManager::absoluteMs() const noexcept {
    return _absoluteMs;
}

gerium_float64_t ServiceManager::elapsed() const noexcept {
    return _elapsed;
}

gerium_float64_t ServiceManager::absolute() const noexcept {
    return _absolute;
}
