#ifndef INPUT_SERVICE_HPP
#define INPUT_SERVICE_HPP

#include "ServiceManager.hpp"

class InputService : public Service {
public:
    bool isPressScancode(gerium_scancode_t scancode) const noexcept;

protected:
    void start() override;
    void stop() override;
    void update() override;

private:
    std::vector<gerium_event_t> _events{};
};

#endif
