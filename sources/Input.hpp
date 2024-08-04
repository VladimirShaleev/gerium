#ifndef GERIUM_INPUT_HPP
#define GERIUM_INPUT_HPP

#include "ObjectPtr.hpp"

namespace gerium {

class Application;

class Input : public gerium::Object {
public:
    void poll();

    bool isPressScancode(gerium_scancode_t scancode) const noexcept;

    static ObjectPtr<Input> create(Application* application);

private:
    virtual void onPoll() = 0;

    virtual bool onIsPressScancode(gerium_scancode_t scancode) const noexcept = 0;
};

} // namespace gerium

#endif
