#include "CommandBuffer.hpp"

namespace gerium {

void CommandBuffer::bindRenderer(Renderer* renderer) noexcept {
    _renderer = renderer;
}

void CommandBuffer::bindMaterial(MaterialHandle handle) noexcept {
    onBindMaterial(handle);
}

void CommandBuffer::bindVertexBuffer(BufferHandle handle, gerium_uint32_t binding, gerium_uint32_t offset) noexcept {
    onBindVertexBuffer(handle, binding, offset);
}

void CommandBuffer::draw(gerium_uint32_t firstVertex,
                         gerium_uint32_t vertexCount,
                         gerium_uint32_t firstInstance,
                         gerium_uint32_t instanceCount) noexcept {
    onDraw(firstVertex, vertexCount, firstInstance, instanceCount);
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

void gerium_command_buffer_draw(gerium_command_buffer_t command_buffer,
                                gerium_uint32_t first_vertex,
                                gerium_uint32_t vertex_count,
                                gerium_uint32_t first_instance,
                                gerium_uint32_t instance_count) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->draw(first_vertex, vertex_count, first_instance, instance_count);
}
