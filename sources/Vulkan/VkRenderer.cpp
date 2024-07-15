#include "VkRenderer.hpp"

namespace gerium::vulkan {

VkRenderer::VkRenderer(Application* application, std::unique_ptr<Device>&& device) noexcept :
    _application(application),
    _device(std::move(device)) {
    if (!application->isRunning()) {
        error(GERIUM_RESULT_ERROR_APPLICATION_NOT_RUNNING);
    }
}

void VkRenderer::onInitialize(gerium_uint32_t version, bool debug) {
    _device->create(application(), version, debug);
}

Application* VkRenderer::application() noexcept {
    return _application.get();
}

BufferHandle VkRenderer::onCreateBuffer(const gerium_buffer_creation_t& creation) {
    BufferCreation bc;
    // TODO: add creation
    return _device->createBuffer(bc);
}

TextureHandle VkRenderer::onCreateTexture(const TextureCreation& creation) {
    return _device->createTexture(creation);
}

void VkRenderer::onDestroyTexture(TextureHandle handle) noexcept {
    _device->destroyTexture(handle);
}

bool VkRenderer::onNewFrame() {
    return _device->newFrame();
}

void VkRenderer::onPresent() {
    _device->present();
}

Profiler* VkRenderer::onGetProfiler() noexcept {
    return _device->profiler();
}

} // namespace gerium::vulkan
