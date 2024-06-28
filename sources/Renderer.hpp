#ifndef GERIUM_RENDERER_HPP
#define GERIUM_RENDERER_HPP

#include "ObjectPtr.hpp"

struct _gerium_renderer : public gerium::Object {};

namespace gerium {

class Renderer : public _gerium_renderer {
public:
    Renderer() noexcept;

    gerium_result_t initialize() noexcept;

protected:
    virtual void onInitialize() = 0;
};

} // namespace gerium

#endif
