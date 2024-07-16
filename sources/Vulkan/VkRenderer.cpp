#include "VkRenderer.hpp"
#include "../FrameGraph.hpp"

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

RenderPassHandle VkRenderer::onCreateRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node) {
    RenderPassCreation creation{};
    creation.setName(node->name);

    for (gerium_uint32_t i = 0; i < node->outputCount; ++i) {
        auto& info = frameGraph.getResource(node->outputs[i])->info;

        if (info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT) {
            const auto format = toVkFormat(info.texture.format);

            if (hasDepthOrStencil(format)) {
                creation.output.depth(format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
                creation.output.setDepthStencilOperations(info.texture.operation, info.texture.operation);
            } else {
                creation.output.color(format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, info.texture.operation);
            }
        }
    }

    for (gerium_uint32_t i = 0; i < node->inputCount; ++i) {
        auto& info = frameGraph.getResource(node->inputs[i])->info;

        if (info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT) {
            const auto format = toVkFormat(info.texture.format);

            if (hasDepthOrStencil(format)) {
                creation.output.depth(format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
                creation.output.setDepthStencilOperations(info.texture.operation, GERIUM_RENDER_PASS_OPERATION_LOAD);
            } else {
                creation.output.color(
                    format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, GERIUM_RENDER_PASS_OPERATION_LOAD);
            }
        }
    }

    return _device->createRenderPass(creation);
}

FramebufferHandle VkRenderer::onCreateFramebuffer(const FrameGraph& frameGraph, const FrameGraphNode* node) {
    FramebufferCreation creation{};
    creation.renderPass = node->renderPass;
    creation.setName(node->name);

    gerium_uint16_t width  = 0;
    gerium_uint16_t height = 0;

    for (gerium_uint32_t i = 0; i < node->outputCount; ++i) {
        auto& info = frameGraph.getResource(node->outputs[i])->info;

        if (info.type == GERIUM_RESOURCE_TYPE_BUFFER || info.type == GERIUM_RESOURCE_TYPE_REFERENCE) {
            continue;
        }

        if (width == 0) {
            width = info.texture.width;
        } else {
            assert(width == info.texture.width);
        }

        if (height == 0) {
            height = info.texture.height;
        } else {
            assert(height == info.texture.height);
        }

        if (hasDepthOrStencil(toVkFormat(info.texture.format))) {
            // TODO: check 1 depth/stencil target
            creation.setDepthStencilTexture(info.texture.handle);
        } else {
            creation.addRenderTexture(info.texture.handle);
        }
    }

    for (gerium_uint32_t i = 0; i < node->inputCount; ++i) {
        auto inputResource = frameGraph.getResource(node->inputs[i]);

        if (inputResource->info.type == GERIUM_RESOURCE_TYPE_BUFFER ||
            inputResource->info.type == GERIUM_RESOURCE_TYPE_REFERENCE) {
            continue;
        }

        auto& info = frameGraph.getResource(inputResource->name)->info;

        if (width == 0) {
            width = info.texture.width;
        } else {
            assert(width == info.texture.width);
        }

        if (height == 0) {
            height = info.texture.height;
        } else {
            assert(height == info.texture.height);
        }

        if (inputResource->info.type == GERIUM_RESOURCE_TYPE_TEXTURE) {
            continue;
        }

        if (hasDepthOrStencil(toVkFormat(info.texture.format))) {
            // TODO: check 1 depth/stencil target
            creation.setDepthStencilTexture(info.texture.handle);
        } else {
            creation.addRenderTexture(info.texture.handle);
        }
    }

    creation.width  = width;
    creation.height = height;

    return _device->createFramebuffer(creation);
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
