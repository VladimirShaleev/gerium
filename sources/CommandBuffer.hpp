#ifndef GERIUM_COMMAND_BUFFER_HPP
#define GERIUM_COMMAND_BUFFER_HPP

#include "Handles.hpp"
#include "ObjectPtr.hpp"

struct _gerium_command_buffer : public gerium::Object {};

namespace gerium {

class Renderer;

class CommandBuffer : public _gerium_command_buffer {
public:
    void bindRenderer(Renderer* renderer) noexcept;

    void clearColor(gerium_uint32_t index,
                    gerium_float32_t red,
                    gerium_float32_t green,
                    gerium_float32_t blue,
                    gerium_float32_t alpha) noexcept;
    void clearDepthStencil(gerium_float32_t depth, gerium_uint32_t value) noexcept;
    void bindMaterial(MaterialHandle handle) noexcept;
    void bindVertexBuffer(BufferHandle handle, gerium_uint32_t binding, gerium_uint32_t offset) noexcept;
    void bindDescriptorSet(DescriptorSetHandle handle, gerium_uint32_t set) noexcept;
    void draw(gerium_uint32_t firstVertex,
              gerium_uint32_t vertexCount,
              gerium_uint32_t firstInstance,
              gerium_uint32_t instanceCount) noexcept;
    void drawProfiler(bool* show) noexcept;

protected:
    Renderer* getRenderer() noexcept;

private:
    virtual void onClearColor(gerium_uint32_t index,
                              gerium_float32_t red,
                              gerium_float32_t green,
                              gerium_float32_t blue,
                              gerium_float32_t alpha) noexcept                               = 0;
    virtual void onClearDepthStencil(gerium_float32_t depth, gerium_uint32_t value) noexcept = 0;

    virtual void onBindMaterial(MaterialHandle handle) noexcept                                                    = 0;
    virtual void onBindVertexBuffer(BufferHandle handle, gerium_uint32_t binding, gerium_uint32_t offset) noexcept = 0;
    virtual void onBindDescriptorSet(DescriptorSetHandle handle, gerium_uint32_t set) noexcept                     = 0;

    virtual void onDraw(gerium_uint32_t firstVertex,
                        gerium_uint32_t vertexCount,
                        gerium_uint32_t firstInstance,
                        gerium_uint32_t instanceCount) noexcept = 0;

    Renderer* _renderer{};
};

} // namespace gerium

#endif
