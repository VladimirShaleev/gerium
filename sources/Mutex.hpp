#ifndef GERIUM_MUTEX_HPP
#define GERIUM_MUTEX_HPP

#include "ObjectPtr.hpp"

struct _gerium_mutex : public gerium::Object {};

namespace gerium {

class Mutex : public _gerium_mutex {
public:
    void lock() noexcept;
    void unlock() noexcept;

private:
    marl::mutex _mutex;
};

} // namespace gerium

#endif
