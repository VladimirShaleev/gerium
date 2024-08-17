#include "Mutex.hpp"

namespace gerium {

void Mutex::lock() noexcept {
    _mutex.lock();
}

void Mutex::unlock() noexcept {
    _mutex.unlock();
}

} // namespace gerium

using namespace gerium;

gerium_result_t gerium_mutex_create(gerium_mutex_t* mutex) {
    assert(mutex);
    return Object::create<Mutex>(*mutex);
}

gerium_mutex_t gerium_mutex_reference(gerium_mutex_t mutex) {
    assert(mutex);
    mutex->reference();
    return mutex;
}

void gerium_mutex_destroy(gerium_mutex_t mutex) {
    if (mutex) {
        mutex->destroy();
    }
}

void gerium_mutex_lock(gerium_mutex_t mutex) {
    assert(mutex);
    alias_cast<Mutex*>(mutex)->lock();
}

void gerium_mutex_unlock(gerium_mutex_t mutex) {
    assert(mutex);
    alias_cast<Mutex*>(mutex)->unlock();
}
