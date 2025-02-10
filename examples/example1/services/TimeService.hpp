#ifndef TIME_SERVICE_HPP
#define TIME_SERVICE_HPP

#include "ServiceManager.hpp"

class TimeService : public Service {
public:
    void setMultiply(gerium_float64_t multiply) noexcept;
    [[nodiscard]] gerium_float64_t multiply() const noexcept;

    [[nodiscard]] gerium_uint64_t elapsedMs() const noexcept;
    [[nodiscard]] gerium_uint64_t absoluteMs() const noexcept;
    [[nodiscard]] gerium_float64_t elapsed() const noexcept;
    [[nodiscard]] gerium_float64_t absolute() const noexcept;

protected:
    void start() override;
    void stop() override;
    void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) override;

private:
    gerium_float64_t _multiply{ 1.0 };
    gerium_uint64_t _elapsedMs{};
    gerium_uint64_t _absoluteMs{};
    gerium_float64_t _elapsed{};
    gerium_float64_t _absolute{};
};

#endif
