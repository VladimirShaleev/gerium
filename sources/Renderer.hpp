#ifndef GERIUM_RENDERER_HPP
#define GERIUM_RENDERER_HPP

#include "Handles.hpp"
#include "ObjectPtr.hpp"

struct _gerium_renderer : public gerium::Object {};

namespace gerium {

class Renderer : public _gerium_renderer {
public:
    Renderer() noexcept;

    gerium_result_t initialize(gerium_uint32_t version, bool debug) noexcept;

    gerium_result_t createTexture(const gerium_texture_creation_t& creation, gerium_texture_h& handle) noexcept;

    gerium_result_t newFrame() noexcept;

    gerium_result_t present() noexcept;

protected:
    virtual void onInitialize(gerium_uint32_t version, bool debug) = 0;

private:
    virtual TextureHandle onCreateTexture(const gerium_texture_creation_t& creation) noexcept = 0;

    virtual void onNewFrame() = 0;
    virtual void onPresent()  = 0;
};

} // namespace gerium

#endif
