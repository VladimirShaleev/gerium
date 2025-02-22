#ifndef SERVICE_MANAGER_HPP
#define SERVICE_MANAGER_HPP

#include "../Common.hpp"

class Application;
class ServiceManager;

class Service {
public:
    virtual ~Service() = default;

    [[nodiscard]] ServiceManager& manager() noexcept;
    [[nodiscard]] const ServiceManager& manager() const noexcept;

    [[nodiscard]] Application& application() noexcept;
    [[nodiscard]] const Application& application() const noexcept;

    [[nodiscard]] entt::registry& entityRegistry() noexcept;
    [[nodiscard]] const entt::registry& entityRegistry() const noexcept;

    [[nodiscard]] bool isStopped() const noexcept;

protected:
    virtual void start();
    virtual void stop();
    virtual void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) = 0;

    virtual entt::hashed_string stateName() const noexcept;
    virtual std::vector<gerium_uint8_t> saveState();
    virtual void restoreState(const std::vector<gerium_uint8_t>& data);

private:
    friend ServiceManager;

    void setManager(ServiceManager* manager, uint32_t id) noexcept;
    void setStopped(bool stop);
    uint32_t serviceId() const noexcept;

    ServiceManager* _manager{};
    uint32_t _id{};
    int _started{};
};

class ServiceManager final : NonMovable {
public:
    void create(Application* application);
    void destroy() noexcept;

    void update(gerium_uint64_t elapsedMs);

    std::map<entt::hashed_string, std::vector<uint8_t>> saveState();
    void restoreState(const std::map<hashed_string_owner, std::vector<uint8_t>>& states);

    [[nodiscard]] Application& application() noexcept;
    [[nodiscard]] const Application& application() const noexcept;

    [[nodiscard]] gerium_uint64_t elapsedMs() const noexcept;
    [[nodiscard]] gerium_uint64_t absoluteMs() const noexcept;
    [[nodiscard]] gerium_float64_t elapsed() const noexcept;
    [[nodiscard]] gerium_float64_t absolute() const noexcept;

    template <typename T>
    [[nodiscard]] T* getService() noexcept {
        if (auto it = _hashServices.find(entt::type_index<T>::value()); it != _hashServices.end()) {
            return (T*) it->second;
        }
        return nullptr;
    }

    template <typename T>
    [[nodiscard]] std::vector<T*> getServices() noexcept {
        auto [hashBegin, hashEnd] = _hashServices.equal_range(entt::type_index<T>::value());

        std::vector<T*> results;
        for (auto it = hashBegin; it != hashEnd; ++it) {
            results.push_back((T*) it->second);
        }
        return results;
    }

    template <typename T, typename... Args>
    T* addService(Args&&... args) {
        static_assert(std::is_base_of_v<Service, T>, "T must inheritance from Service");

        auto service = std::make_unique<T>(std::forward<Args>(args)...);
        auto result  = service.get();
        result->setManager(this, entt::type_index<T>::value());
        startService(result);

        _services.push_back(std::move(service));
        _hashServices.insert({ entt::type_index<T>::value(), result });

        return result;
    }

    void startService(Service* service) {
        try {
            service->setStopped(false);
        } catch (...) {
            service->setStopped(true);
            throw;
        }
    }

    void stopService(Service* service) noexcept {
        service->setStopped(true);
    }

    bool removeService(Service* service) {
        auto it = std::find_if(_services.begin(), _services.end(), [service](const auto& item) {
            return item.get() == service;
        });

        if (it != _services.end()) {
            auto [hashBegin, hashEnd] = _hashServices.equal_range(service->serviceId());

            auto hashIt = std::find_if(hashBegin, hashEnd, [service](const auto& item) {
                return item.second == service;
            });
            assert(hashIt != hashEnd);
            _services.erase(it);
            _hashServices.erase(hashIt);
            return true;
        }
        return false;
    }

private:
    Application* _application{};
    gerium_uint64_t _elapsedMs{};
    gerium_uint64_t _absoluteMs{};
    gerium_float64_t _elapsed{};
    gerium_float64_t _absolute{};
    std::vector<std::unique_ptr<Service>> _services{};
    std::multimap<uint32_t, Service*> _hashServices{};
};

#endif
