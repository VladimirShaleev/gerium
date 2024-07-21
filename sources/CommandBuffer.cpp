#include "CommandBuffer.hpp"
#include "ProfilerUI.hpp"
#include "Renderer.hpp"

namespace gerium {

void CommandBuffer::bindRenderer(Renderer* renderer) noexcept {
    _renderer = renderer;
}

void CommandBuffer::clearColor(gerium_uint32_t index,
                               gerium_float32_t red,
                               gerium_float32_t green,
                               gerium_float32_t blue,
                               gerium_float32_t alpha) noexcept {
    onClearColor(index, red, green, blue, alpha);
}

void CommandBuffer::clearDepthStencil(gerium_float32_t depth, gerium_uint32_t value) noexcept {
    onClearDepthStencil(depth, value);
}

void CommandBuffer::bindMaterial(MaterialHandle handle) noexcept {
    onBindMaterial(handle);
}

void CommandBuffer::bindVertexBuffer(BufferHandle handle, gerium_uint32_t binding, gerium_uint32_t offset) noexcept {
    onBindVertexBuffer(handle, binding, offset);
}

void CommandBuffer::bindDescriptorSet(DescriptorSetHandle handle, gerium_uint32_t set) noexcept {
    onBindDescriptorSet(handle, set);
}

void CommandBuffer::draw(gerium_uint32_t firstVertex,
                         gerium_uint32_t vertexCount,
                         gerium_uint32_t firstInstance,
                         gerium_uint32_t instanceCount) noexcept {
    onDraw(firstVertex, vertexCount, firstInstance, instanceCount);
}

void CommandBuffer::drawProfiler(bool* show) noexcept {
    static ProfilerUI profilerUI;
    profilerUI.draw(getRenderer()->getProfiler(), show, 100);
}

Renderer* CommandBuffer::getRenderer() noexcept {
    return _renderer;
}

} // namespace gerium

using namespace gerium;

void gerium_command_buffer_bind_material(gerium_command_buffer_t command_buffer, gerium_material_h handle) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->bindMaterial({ handle.unused });
}

void gerium_command_buffer_bind_vertex_buffer(gerium_command_buffer_t command_buffer,
                                              gerium_buffer_h handle,
                                              gerium_uint32_t binding,
                                              gerium_uint32_t offset) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->bindVertexBuffer({ handle.unused }, binding, offset);
}

void gerium_command_buffer_bind_descriptor_set(gerium_command_buffer_t command_buffer,
                                               gerium_descriptor_set_h handle,
                                               gerium_uint32_t set) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->bindDescriptorSet({ handle.unused }, set);
}

void gerium_command_buffer_draw(gerium_command_buffer_t command_buffer,
                                gerium_uint32_t first_vertex,
                                gerium_uint32_t vertex_count,
                                gerium_uint32_t first_instance,
                                gerium_uint32_t instance_count) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->draw(first_vertex, vertex_count, first_instance, instance_count);
}

void gerium_command_buffer_draw_profiler(gerium_command_buffer_t command_buffer, gerium_bool_t* show) {
    assert(command_buffer);
    bool bShow  = show ? *show : true;
    bool* pShow = show ? &bShow : nullptr;
    alias_cast<CommandBuffer*>(command_buffer)->drawProfiler(pShow);
    if (show) {
        *show = bShow;
    }
}
