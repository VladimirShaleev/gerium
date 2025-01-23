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

EventManager& Service::eventManager() noexcept {
    return application().eventManager();
}

const EventManager& Service::eventManager() const noexcept {
    return application().eventManager();
}

bool Service::isStopped() const noexcept {
    return _started == 0;
}

void Service::start() {
}

void Service::stop() {
}

void Service::update() {
}

void Service::setManager(ServiceManager* manager, int id) noexcept {
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

int Service::serviceId() const noexcept {
    return _id;
}

void ServiceManager::create(Application* application) {
    assert(application);
    _application = application;
}

void ServiceManager::destroy() noexcept {
    for (auto it = _services.begin(); it != _services.end(); ++it) {
        auto service = it->get();
        service->setStopped(true);
    }
    _hashServices.clear();
    _services.clear();
}

void ServiceManager::update(gerium_uint64_t elapsedMs) {
    _elapsedMs = elapsedMs;
    _absoluteMs += elapsedMs;

    for (auto it = _services.begin(); it != _services.end(); ++it) {
        auto service = it->get();
        if (!service->isStopped()) {
            service->update();
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
