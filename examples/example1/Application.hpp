#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "ResourceManager.hpp"
#include "components/ECS.hpp"
#include "events/EventManager.hpp"
#include "passes/RenderPass.hpp"
#include "services/ServiceManager.hpp"

class Application final {
public:
    Application();
    ~Application();

    void run(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) noexcept;

    gerium_application_t handle() const noexcept {
        return _application;
    }

    gerium_logger_t logger() const noexcept {
        return _logger;
    }

    EventManager& eventManager() noexcept {
        return _eventManager;
    }

    const EventManager& eventManager() const noexcept {
        return _eventManager;
    }

    EntityManager& entityManager() noexcept {
        return _entityManager;
    }

    const EntityManager& entityManager() const noexcept {
        return _entityManager;
    }

private:
    void initialize();
    void uninitialize();

    void frame(gerium_uint64_t elapsedMs);
    void state(gerium_application_state_t state);

    static gerium_bool_t frame(gerium_application_t application, gerium_data_t data, gerium_uint64_t elapsedMs);
    static gerium_bool_t state(gerium_application_t application, gerium_data_t data, gerium_application_state_t state);

    std::exception_ptr _error{};
    gerium_logger_t _logger{};
    gerium_application_t _application{};
    ServiceManager _serviceManager{};
    EventManager _eventManager{};
    EntityManager _entityManager{};
};

#endif
