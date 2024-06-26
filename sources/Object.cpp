#include "Object.hpp"

namespace gerium {

Object::Object() noexcept : _refCount(1) {
}

void Object::reference() noexcept {
    ++_refCount;
}

void Object::destroy() noexcept {
    if (--_refCount == 0) {
        delete this;
    }
}

} // namespace gerium
