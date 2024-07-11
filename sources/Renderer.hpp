#ifndef GERIUM_RENDERER_HPP
#define GERIUM_RENDERER_HPP

#include "Handles.hpp"
#include "ObjectPtr.hpp"
#include "Profiler.hpp"

struct _gerium_renderer : public gerium::Object {};

namespace gerium {

class Renderer : public _gerium_renderer {
public:
    Renderer() noexcept;

    gerium_result_t initialize(gerium_uint32_t version, bool debug) noexcept;

    gerium_result_t createBuffer(const gerium_buffer_creation_t& creation, gerium_buffer_h& handle) noexcept;
    gerium_result_t createTexture(const gerium_texture_creation_t& creation, gerium_texture_h& handle) noexcept;

    void destroyTexture(gerium_texture_h handle) noexcept;

    gerium_result_t newFrame() noexcept;

    gerium_result_t present() noexcept;

    Profiler* getProfiler() noexcept;

protected:
    virtual void onInitialize(gerium_uint32_t version, bool debug) = 0;

private:
    virtual BufferHandle onCreateBuffer(const gerium_buffer_creation_t& creation) = 0;
    virtual TextureHandle onCreateTexture(const gerium_texture_creation_t& creation) = 0;

    virtual void onDestroyTexture(TextureHandle handle) noexcept = 0;

    virtual bool onNewFrame() = 0;
    virtual void onPresent()  = 0;

    virtual Profiler* onGetProfiler() noexcept = 0;
};

} // namespace gerium

#endif
