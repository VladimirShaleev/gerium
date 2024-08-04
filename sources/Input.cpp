#include "Input.hpp"

namespace gerium {

void Input::poll() {
    onPoll();
}

bool Input::isPressScancode(gerium_scancode_t scancode) const noexcept {
    return onIsPressScancode(scancode);
}

} // namespace gerium
