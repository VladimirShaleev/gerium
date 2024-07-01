#ifndef GERIUM_RENDERER_HPP
#define GERIUM_RENDERER_HPP

#include "ObjectPtr.hpp"

struct _gerium_renderer : public gerium::Object {};

namespace gerium {

class Renderer : public _gerium_renderer {
public:
    Renderer() noexcept;

    gerium_result_t initialize(gerium_uint32_t version, bool debug) noexcept;

protected:
    virtual void onInitialize(gerium_uint32_t version, bool debug) = 0;
};

} // namespace gerium

#endif
