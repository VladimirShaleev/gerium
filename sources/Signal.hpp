#ifndef GERIUM_SIGNAL_HPP
#define GERIUM_SIGNAL_HPP

#include "ObjectPtr.hpp"

struct _gerium_signal : public gerium::Object {};

namespace gerium {

class Signal final : public _gerium_signal {
public:
    Signal() noexcept;
    
    bool isSet() const noexcept;
    
    void set() noexcept;
    void wait() noexcept;
    void clear() noexcept;

private:
    marl::Event _event;
};

} // namespace gerium

#endif
