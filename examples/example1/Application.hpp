#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "ResourceManager.hpp"
#include "passes/RenderPass.hpp"
#include "services/ServiceManager.hpp"

class Application final {
public:
    Application();
    ~Application();

    void run(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) noexcept;

    [[nodiscard]] gerium_application_t handle() const noexcept;
    [[nodiscard]] gerium_logger_t logger() const noexcept;

    [[nodiscard]] entt::registry& entityRegistry() noexcept;
    [[nodiscard]] const entt::registry& entityRegistry() const noexcept;

    [[nodiscard]] entt::dispatcher& dispatcher() noexcept;
    [[nodiscard]] const entt::dispatcher& dispatcher() const noexcept;

private:
    void addModel(const hashed_string_owner& name,
                  const entt::hashed_string& model,
                  const glm::vec3& position = { 0.0f, 0.0f, 0.0f },
                  const glm::quat& rotation = { 1.0f, 0.0f, 0.0f, 0.0f },
                  const glm::vec3& scale    = { 1.0f, 1.0f, 1.0f });

    void initialize();
    void uninitialize();

    void saveState();
    void loadState();

    void frame(gerium_uint64_t elapsedMs);
    void state(gerium_application_state_t state);

    static gerium_bool_t frame(gerium_application_t application, gerium_data_t data, gerium_uint64_t elapsedMs);
    static gerium_bool_t state(gerium_application_t application, gerium_data_t data, gerium_application_state_t state);

    std::exception_ptr _error{};
    gerium_logger_t _logger{};
    gerium_application_t _application{};
    ServiceManager _serviceManager{};
    entt::registry _entityRegistry{};
    entt::dispatcher _dispatcher{};
};

inline gerium_application_t Application::handle() const noexcept {
    return _application;
}

inline gerium_logger_t Application::logger() const noexcept {
    return _logger;
}

inline entt::registry& Application::entityRegistry() noexcept {
    return _entityRegistry;
}

inline const entt::registry& Application::entityRegistry() const noexcept {
    return _entityRegistry;
}

inline entt::dispatcher& Application::dispatcher() noexcept {
    return _dispatcher;
}

inline const entt::dispatcher& Application::dispatcher() const noexcept {
    return _dispatcher;
}

#endif
