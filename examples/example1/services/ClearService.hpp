#ifndef SERVICES_CLEAR_SERVICE_HPP
#define SERVICES_CLEAR_SERVICE_HPP

#include "ServiceManager.hpp"

class ClearService final : public Service {
private:
    void update(gerium_uint64_t /* elapsedMs */, gerium_float64_t /* elapsed */) override;

    template <typename C>
    void clearChanges(Change& change) noexcept {
        if (change != Change::None) {
            change = Change::None;
            for (auto [_, component] : entityRegistry().view<C>().each()) {
                component.changed = false;
            }
        }
    }
};

#endif
