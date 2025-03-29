#ifndef SERVICE_MANAGER_HPP
#define SERVICE_MANAGER_HPP

#include "../Common.hpp"

class Application;
class ServiceManager;

class Service : NonMovable {
public:
    virtual ~Service() = default;

    [[nodiscard]] ServiceManager& manager() noexcept;
    [[nodiscard]] const ServiceManager& manager() const noexcept;

    [[nodiscard]] Application& application() noexcept;
    [[nodiscard]] const Application& application() const noexcept;

    [[nodiscard]] entt::registry& entityRegistry() noexcept;
    [[nodiscard]] const entt::registry& entityRegistry() const noexcept;

    [[nodiscard]] entt::dispatcher& dispatcher() noexcept;
    [[nodiscard]] const entt::dispatcher& dispatcher() const noexcept;

protected:
    virtual void start();
    virtual void stop();
    virtual void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) = 0;

    virtual entt::hashed_string stateName() const noexcept;
    virtual std::vector<gerium_uint8_t> saveState();
    virtual void restoreState(const std::vector<gerium_uint8_t>& data);

private:
    friend ServiceManager;

    void setManager(ServiceManager* manager, gerium_uint32_t id) noexcept;
    gerium_uint32_t serviceId() const noexcept;

    ServiceManager* _manager{};
    gerium_uint32_t _id{};
};

class ServiceManager final : NonMovable {
public:
    void create(Application* application);
    void destroy() noexcept;

    void update(gerium_uint64_t elapsedMs);

    [[nodiscard]] std::map<entt::hashed_string, std::vector<uint8_t>> saveState();
    void restoreState(const std::map<hashed_string_owner, std::vector<uint8_t>>& states);

    [[nodiscard]] Application& application() noexcept;
    [[nodiscard]] const Application& application() const noexcept;

    [[nodiscard]] gerium_uint64_t elapsedMs() const noexcept;
    [[nodiscard]] gerium_uint64_t absoluteMs() const noexcept;
    [[nodiscard]] gerium_float64_t elapsed() const noexcept;
    [[nodiscard]] gerium_float64_t absolute() const noexcept;

    template <typename T>
    [[nodiscard]] T* getService() noexcept {
        const auto serviceId = getId<T>();
        if (auto it = _hashServices.find(serviceId); it != _hashServices.end()) {
            return (T*) it->second;
        }
        return nullptr;
    }

    template <typename T, typename... Args>
    T* addService(Args&&... args) {
        const auto serviceId = getId<T>();

        auto service = std::make_unique<T>(std::forward<Args>(args)...);
        auto result  = service.get();
        static_cast<Service*>(result)->setManager(this, serviceId);
        static_cast<Service*>(result)->start();

        _services.push_back(std::move(service));
        _hashServices.insert({ serviceId, result });
        return result;
    }

    bool removeService(Service* service) {
        auto it = std::find_if(_services.begin(), _services.end(), [service](const auto& item) {
            return item.get() == service;
        });

        if (it != _services.end()) {
            _hashServices.erase(_hashServices.find((*it)->serviceId()));
            _services.erase(it);
            return true;
        }
        return false;
    }

private:
    template <typename T>
    static gerium_uint32_t getId() noexcept {
        static_assert(std::is_base_of_v<Service, T>, "T must inheritance from Service");
        return entt::type_index<T>::value();
    }

    Application* _application{};
    gerium_uint64_t _elapsedMs{};
    gerium_uint64_t _absoluteMs{};
    gerium_float64_t _elapsed{};
    gerium_float64_t _absolute{};
    std::vector<std::unique_ptr<Service>> _services{};
    std::map<gerium_uint32_t, Service*> _hashServices{};
};

#endif
