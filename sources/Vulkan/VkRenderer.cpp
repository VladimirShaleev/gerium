#include "VkRenderer.hpp"

namespace gerium::vulkan {

VkRenderer::VkRenderer(Application* application, std::unique_ptr<Device>&& device) noexcept :
    _application(application),
    _device(std::move(device)) {
}

void VkRenderer::onInitialize(gerium_uint32_t version, bool debug) {
    _device->create(application(), version, debug);
}

Application* VkRenderer::application() noexcept {
    return _application.get();
}

TextureHandle VkRenderer::onCreateTexture(const gerium_texture_creation_t& creation) {
    TextureCreation tc;
    tc.setSize(creation.width, creation.height, creation.depth)
        .setFlags(creation.mipmaps, false, false)
        .setFormat(creation.format, creation.type)
        .setData((void*) creation.data)
        .setName(creation.name);
    return _device->createTexture(tc);
}

void VkRenderer::onDestroyTexture(TextureHandle handle) noexcept {
    _device->destroyTexture(handle);
}

void VkRenderer::onNewFrame() {
    _device->newFrame();
}

void VkRenderer::onPresent() {
    _device->present();
}

} // namespace gerium::vulkan
