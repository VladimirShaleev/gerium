#ifndef GERIUM_RENDERER_HPP
#define GERIUM_RENDERER_HPP

#include "Object.hpp"

struct _gerium_renderer : public gerium::Object {};

namespace gerium {

class Renderer : public _gerium_renderer {
public:
    Renderer() noexcept;
};

} // namespace gerium

#endif
