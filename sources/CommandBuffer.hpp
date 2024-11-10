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

    void setViewport(gerium_uint16_t x,
                     gerium_uint16_t y,
                     gerium_uint16_t width,
                     gerium_uint16_t height,
                     gerium_float32_t minDepth,
                     gerium_float32_t maxDepth) noexcept;
    void setScissor(gerium_uint16_t x, gerium_uint16_t y, gerium_uint16_t width, gerium_uint16_t height) noexcept;
    void bindTechnique(TechniqueHandle handle) noexcept;
    void bindVertexBuffer(BufferHandle handle, gerium_uint32_t binding, gerium_uint32_t offset) noexcept;
    void bindIndexBuffer(BufferHandle handle, gerium_uint32_t offset, gerium_index_type_t type) noexcept;
    void bindDescriptorSet(DescriptorSetHandle handle, gerium_uint32_t set) noexcept;
    void dispatch(gerium_uint32_t groupX, gerium_uint32_t groupY, gerium_uint32_t groupZ) noexcept;
    void draw(gerium_uint32_t firstVertex,
              gerium_uint32_t vertexCount,
              gerium_uint32_t firstInstance,
              gerium_uint32_t instanceCount) noexcept;
    void drawIndexed(gerium_uint32_t firstIndex,
                     gerium_uint32_t indexCount,
                     gerium_uint32_t vertexOffset,
                     gerium_uint32_t firstInstance,
                     gerium_uint32_t instanceCount) noexcept;
    void drawMeshTasks(gerium_uint32_t groupX, gerium_uint32_t groupY, gerium_uint32_t groupZ) noexcept;
    void drawMeshTasksIndirect(BufferHandle handle,
                               gerium_uint32_t offset,
                               gerium_uint32_t drawCount,
                               gerium_uint32_t stride) noexcept;
    void drawProfiler(bool* show) noexcept;
    void fillBuffer(BufferHandle handle, gerium_uint32_t offset, gerium_uint32_t size, gerium_uint32_t data) noexcept;
    void barrierBufferWrite(BufferHandle handle) noexcept;
    void barrierBufferRead(BufferHandle handle) noexcept;
    void barrierTextureWrite(TextureHandle handle) noexcept;
    void barrierTextureRead(TextureHandle handle) noexcept;
    FfxCommandList getFfxCommandList() noexcept;

protected:
    Renderer* getRenderer() noexcept;

private:
    virtual void onSetViewport(gerium_uint16_t x,
                               gerium_uint16_t y,
                               gerium_uint16_t width,
                               gerium_uint16_t height,
                               gerium_float32_t minDepth,
                               gerium_float32_t maxDepth) noexcept = 0;
    virtual void onSetScissor(gerium_uint16_t x,
                              gerium_uint16_t y,
                              gerium_uint16_t width,
                              gerium_uint16_t height) noexcept     = 0;

    virtual void onBindTechnique(TechniqueHandle handle) noexcept                                                  = 0;
    virtual void onBindVertexBuffer(BufferHandle handle, gerium_uint32_t binding, gerium_uint32_t offset) noexcept = 0;
    virtual void onBindIndexBuffer(BufferHandle handle, gerium_uint32_t offset, gerium_index_type_t type) noexcept = 0;
    virtual void onBindDescriptorSet(DescriptorSetHandle handle, gerium_uint32_t set) noexcept                     = 0;

    virtual void onDispatch(gerium_uint32_t groupX, gerium_uint32_t groupY, gerium_uint32_t groupZ) noexcept = 0;

    virtual void onDraw(gerium_uint32_t firstVertex,
                        gerium_uint32_t vertexCount,
                        gerium_uint32_t firstInstance,
                        gerium_uint32_t instanceCount) noexcept = 0;

    virtual void onDrawIndexed(gerium_uint32_t firstIndex,
                               gerium_uint32_t indexCount,
                               gerium_uint32_t vertexOffset,
                               gerium_uint32_t firstInstance,
                               gerium_uint32_t instanceCount) noexcept = 0;

    virtual void onDrawMeshTasksIndirect(BufferHandle handle,
                                         gerium_uint32_t offset,
                                         gerium_uint32_t drawCount,
                                         gerium_uint32_t stride) noexcept = 0;

    virtual void onDrawMeshTasks(gerium_uint32_t groupX, gerium_uint32_t groupY, gerium_uint32_t groupZ) noexcept = 0;

    virtual void onFillBuffer(BufferHandle handle,
                              gerium_uint32_t offset,
                              gerium_uint32_t size,
                              gerium_uint32_t data) noexcept = 0;

    virtual void onBarrierBufferWrite(BufferHandle handle) noexcept   = 0;
    virtual void onBarrierBufferRead(BufferHandle handle) noexcept    = 0;
    virtual void onBarrierTextureWrite(TextureHandle handle) noexcept = 0;
    virtual void onBarrierTextureRead(TextureHandle handle) noexcept  = 0;

    virtual FfxCommandList onGetFfxCommandList() noexcept = 0;

    Renderer* _renderer{};
};

} // namespace gerium

#endif
