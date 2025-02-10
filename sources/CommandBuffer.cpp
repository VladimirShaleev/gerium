#include "CommandBuffer.hpp"
#include "ProfilerUI.hpp"
#include "Renderer.hpp"

namespace gerium {

void CommandBuffer::bindRenderer(Renderer* renderer) noexcept {
    _renderer = renderer;
}

void CommandBuffer::setViewport(gerium_uint16_t x,
                                gerium_uint16_t y,
                                gerium_uint16_t width,
                                gerium_uint16_t height,
                                gerium_float32_t minDepth,
                                gerium_float32_t maxDepth) noexcept {
    onSetViewport(x, y, width, height, minDepth, maxDepth);
}

void CommandBuffer::setScissor(gerium_uint16_t x,
                               gerium_uint16_t y,
                               gerium_uint16_t width,
                               gerium_uint16_t height) noexcept {
    onSetScissor(x, y, width, height);
}

void CommandBuffer::bindTechnique(TechniqueHandle handle) noexcept {
    onBindTechnique(handle);
}

void CommandBuffer::bindVertexBuffer(BufferHandle handle, gerium_uint32_t binding, gerium_uint32_t offset) noexcept {
    onBindVertexBuffer(handle, binding, offset);
}

void CommandBuffer::bindIndexBuffer(BufferHandle handle, gerium_uint32_t offset, gerium_index_type_t type) noexcept {
    onBindIndexBuffer(handle, offset, type);
}

void CommandBuffer::bindDescriptorSet(DescriptorSetHandle handle, gerium_uint32_t set) noexcept {
    onBindDescriptorSet(handle, set);
}

void CommandBuffer::dispatch(gerium_uint32_t groupX, gerium_uint32_t groupY, gerium_uint32_t groupZ) noexcept {
    onDispatch(groupX, groupY, groupZ);
}

void CommandBuffer::draw(gerium_uint32_t firstVertex,
                         gerium_uint32_t vertexCount,
                         gerium_uint32_t firstInstance,
                         gerium_uint32_t instanceCount) noexcept {
    onDraw(firstVertex, vertexCount, firstInstance, instanceCount);
}

void CommandBuffer::drawIndirect(BufferHandle handle,
                                 gerium_uint32_t offset,
                                 BufferHandle drawCountHandle,
                                 gerium_uint32_t drawCountOffset,
                                 gerium_uint32_t drawCount,
                                 gerium_uint32_t stride) noexcept {
    onDrawIndirect(handle, offset, drawCountHandle, drawCountOffset, drawCount, stride);
}

void CommandBuffer::drawIndexed(gerium_uint32_t firstIndex,
                                gerium_uint32_t indexCount,
                                gerium_uint32_t vertexOffset,
                                gerium_uint32_t firstInstance,
                                gerium_uint32_t instanceCount) noexcept {
    onDrawIndexed(firstIndex, indexCount, vertexOffset, firstInstance, instanceCount);
}

void CommandBuffer::drawIndexedIndirect(BufferHandle handle,
                                        gerium_uint32_t offset,
                                        BufferHandle drawCountHandle,
                                        gerium_uint32_t drawCountOffset,
                                        gerium_uint32_t drawCount,
                                        gerium_uint32_t stride) noexcept {
    onDrawIndexedIndirect(handle, offset, drawCountHandle, drawCountOffset, drawCount, stride);
}

void CommandBuffer::drawMeshTasks(gerium_uint32_t groupX, gerium_uint32_t groupY, gerium_uint32_t groupZ) noexcept {
    onDrawMeshTasks(groupX, groupY, groupZ);
}

void CommandBuffer::drawMeshTasksIndirect(BufferHandle handle,
                                          gerium_uint32_t offset,
                                          gerium_uint32_t drawCount,
                                          gerium_uint32_t stride) noexcept {
    onDrawMeshTasksIndirect(handle, offset, drawCount, stride);
}

void CommandBuffer::drawProfiler(bool* show) noexcept {
    static ProfilerUI profilerUI;
    profilerUI.draw(getRenderer()->getProfiler(), show, 100);
}

void CommandBuffer::fillBuffer(BufferHandle handle,
                               gerium_uint32_t offset,
                               gerium_uint32_t size,
                               gerium_uint32_t data) noexcept {
    onFillBuffer(handle, offset, size, data);
}

void CommandBuffer::barrierBufferWrite(BufferHandle handle) noexcept {
    onBarrierBufferWrite(handle);
}

void CommandBuffer::barrierBufferRead(BufferHandle handle) noexcept {
    onBarrierBufferRead(handle);
}

void CommandBuffer::barrierTextureWrite(TextureHandle handle) noexcept {
    onBarrierTextureWrite(handle);
}

void CommandBuffer::barrierTextureRead(TextureHandle handle) noexcept {
    onBarrierTextureRead(handle);
}

FfxCommandList CommandBuffer::getFfxCommandList() noexcept {
    return onGetFfxCommandList();
}

Renderer* CommandBuffer::getRenderer() noexcept {
    return _renderer;
}

} // namespace gerium

using namespace gerium;

void gerium_command_buffer_set_viewport(gerium_command_buffer_t command_buffer,
                                        gerium_uint16_t x,
                                        gerium_uint16_t y,
                                        gerium_uint16_t width,
                                        gerium_uint16_t height,
                                        gerium_float32_t min_depth,
                                        gerium_float32_t max_depth) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->setViewport(x, y, width, height, min_depth, max_depth);
}

void gerium_command_buffer_set_scissor(gerium_command_buffer_t command_buffer,
                                       gerium_uint16_t x,
                                       gerium_uint16_t y,
                                       gerium_uint16_t width,
                                       gerium_uint16_t height) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->setScissor(x, y, width, height);
}

void gerium_command_buffer_bind_technique(gerium_command_buffer_t command_buffer, gerium_technique_h handle) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->bindTechnique({ handle.index });
}

void gerium_command_buffer_bind_vertex_buffer(gerium_command_buffer_t command_buffer,
                                              gerium_buffer_h handle,
                                              gerium_uint32_t binding,
                                              gerium_uint32_t offset) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->bindVertexBuffer({ handle.index }, binding, offset);
}

void gerium_command_buffer_bind_index_buffer(gerium_command_buffer_t command_buffer,
                                             gerium_buffer_h handle,
                                             gerium_uint32_t offset,
                                             gerium_index_type_t type) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->bindIndexBuffer({ handle.index }, offset, type);
}

void gerium_command_buffer_bind_descriptor_set(gerium_command_buffer_t command_buffer,
                                               gerium_descriptor_set_h handle,
                                               gerium_uint32_t set) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->bindDescriptorSet({ handle.index }, set);
}

void gerium_command_buffer_dispatch(gerium_command_buffer_t command_buffer,
                                    gerium_uint32_t group_x,
                                    gerium_uint32_t group_y,
                                    gerium_uint32_t group_z) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->dispatch(group_x, group_y, group_z);
}

void gerium_command_buffer_draw(gerium_command_buffer_t command_buffer,
                                gerium_uint32_t first_vertex,
                                gerium_uint32_t vertex_count,
                                gerium_uint32_t first_instance,
                                gerium_uint32_t instance_count) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->draw(first_vertex, vertex_count, first_instance, instance_count);
}

void gerium_command_buffer_draw_indirect(gerium_command_buffer_t command_buffer,
                                         gerium_buffer_h handle,
                                         gerium_uint32_t offset,
                                         gerium_buffer_h draw_count_handle,
                                         gerium_uint32_t draw_count_offset,
                                         gerium_uint32_t draw_count,
                                         gerium_uint32_t stride) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)
        ->drawIndirect({ handle.index }, offset, { draw_count_handle.index }, draw_count_offset, draw_count, stride);
}

void gerium_command_buffer_draw_indexed(gerium_command_buffer_t command_buffer,
                                        gerium_uint32_t first_index,
                                        gerium_uint32_t index_count,
                                        gerium_uint32_t vertex_offset,
                                        gerium_uint32_t first_instance,
                                        gerium_uint32_t instance_count) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)
        ->drawIndexed(first_index, index_count, vertex_offset, first_instance, instance_count);
}

void gerium_command_buffer_draw_indexed_indirect(gerium_command_buffer_t command_buffer,
                                                 gerium_buffer_h handle,
                                                 gerium_uint32_t offset,
                                                 gerium_buffer_h draw_count_handle,
                                                 gerium_uint32_t draw_count_offset,
                                                 gerium_uint32_t draw_count,
                                                 gerium_uint32_t stride) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)
        ->drawIndexedIndirect(
            { handle.index }, offset, { draw_count_handle.index }, draw_count_offset, draw_count, stride);
}

void gerium_command_buffer_draw_mesh_tasks(gerium_command_buffer_t command_buffer,
                                           gerium_uint32_t group_x,
                                           gerium_uint32_t group_y,
                                           gerium_uint32_t group_z) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->drawMeshTasks(group_x, group_y, group_z);
}

void gerium_command_buffer_draw_mesh_tasks_indirect(gerium_command_buffer_t command_buffer,
                                                    gerium_buffer_h handle,
                                                    gerium_uint32_t offset,
                                                    gerium_uint32_t draw_count,
                                                    gerium_uint32_t stride) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->drawMeshTasksIndirect({ handle.index }, offset, draw_count, stride);
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

void gerium_command_buffer_fill_buffer(gerium_command_buffer_t command_buffer,
                                       gerium_buffer_h handle,
                                       gerium_uint32_t offset,
                                       gerium_uint32_t size,
                                       gerium_uint32_t data) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->fillBuffer({ handle.index }, offset, size, data);
}

void gerium_command_buffer_barrier_buffer_write(gerium_command_buffer_t command_buffer, gerium_buffer_h handle) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->barrierBufferWrite({ handle.index });
}

void gerium_command_buffer_barrier_buffer_read(gerium_command_buffer_t command_buffer, gerium_buffer_h handle) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->barrierBufferRead({ handle.index });
}

void gerium_command_buffer_barrier_texture_write(gerium_command_buffer_t command_buffer, gerium_texture_h handle) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->barrierTextureWrite({ handle.index });
}

void gerium_command_buffer_barrier_texture_read(gerium_command_buffer_t command_buffer, gerium_texture_h handle) {
    assert(command_buffer);
    alias_cast<CommandBuffer*>(command_buffer)->barrierTextureRead({ handle.index });
}

FfxCommandList gerium_command_buffer_get_ffx_command_list(gerium_command_buffer_t command_buffer) {
    assert(command_buffer);
    return alias_cast<CommandBuffer*>(command_buffer)->getFfxCommandList();
}
