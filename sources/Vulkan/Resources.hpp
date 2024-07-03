#ifndef GERIUM_WINDOWS_VULKAN_RESOURCES_HPP
#define GERIUM_WINDOWS_VULKAN_RESOURCES_HPP

#include "../Gerium.hpp"
#include "../Handles.hpp"
#include "Enums.hpp"

namespace gerium::vulkan {

// clang-format off

constexpr uint8_t kMaxImageOutputs = 8;

struct RenderPassHandle : Handle {};

using TexturePool     = ResourcePool<struct Texture, TextureHandle>;
using RenderPassPool  = ResourcePool<struct RenderPass, RenderPassHandle>;
using FramebufferPool = ResourcePool<struct Framebuffer, FramebufferHandle>;

struct TextureCreation {
    uint16_t              width        = 1;
    uint16_t              height       = 1;
    uint16_t              depth        = 1;
    uint16_t              mipmaps      = 1;
    TextureFlags          flags        = TextureFlags::None;
    gerium_format_t       format       = GERIUM_FORMAT_R8G8B8A8_UNORM;
    gerium_texture_type_t type         = GERIUM_TEXTURE_TYPE_2D;
    TextureHandle         alias        = Undefined;
    void*                 initialData  = nullptr;
    const char*           name         = nullptr;
    
    TextureCreation& setSize(uint16_t width, uint16_t height, uint16_t depth) {
        this->width  = width;
        this->height = height;
        this->depth  = depth;
        return *this;
    }

    TextureCreation& setFlags(uint8_t mipmaps, bool renderTarget, bool compute) {
        this->mipmaps = mipmaps;
        this->flags  |= renderTarget ? TextureFlags::RenderTarget : TextureFlags::None;
        this->flags  |= compute ? TextureFlags::Compute : TextureFlags::None;
        return *this;
    }

    TextureCreation& setFormat(gerium_format_t format, gerium_texture_type_t type) {
        this->format = format;
        this->type   = type;
        return *this;
    }

    TextureCreation& setAlias(TextureHandle alias) {
        this->alias = alias;
        return *this;
    }

    TextureCreation& setData(void* data) {
        this->initialData = data;
        return *this;
    }

    TextureCreation& setName(const char* name) {
        this->name = name;
        return *this;
    }
};

struct RenderPassOutput {
    uint32_t            numColorFormats;
    VkFormat            colorFormats[kMaxImageOutputs];
    VkImageLayout       colorFinalLayouts[kMaxImageOutputs];
    RenderPassOperation colorOperations[kMaxImageOutputs];

    VkFormat            depthStencilFormat;
    VkImageLayout       depthStencilFinalLayout;
    RenderPassOperation depthOperation;
    RenderPassOperation stencilOperation;

    RenderPassOutput& color(VkFormat format, VkImageLayout layout, RenderPassOperation loadOp) {
        assert(numColorFormats < kMaxImageOutputs);
        colorFormats[numColorFormats]      = format;
        colorFinalLayouts[numColorFormats] = layout;
        colorOperations[numColorFormats]   = loadOp;
        ++numColorFormats;
        return *this;
    }

    RenderPassOutput& depth(VkFormat format, VkImageLayout layout) {
        depthStencilFormat      = format;
        depthStencilFinalLayout = layout;
        return *this;
    }

    RenderPassOutput& setDepthStencilOperations(RenderPassOperation depth, RenderPassOperation stencil) {
        depthOperation   = depth;
        stencilOperation = stencil;
        return *this;
    }
};

struct RenderPassCreation {
    RenderPassOutput output = {};
    const char*      name   = nullptr;

    RenderPassCreation& setName(const char* name) {
        this->name = name;
        return *this;
    }
};

struct FramebufferCreation {
    RenderPassHandle renderPass;

    uint16_t      numRenderTargets                 = 0;
    TextureHandle outputTextures[kMaxImageOutputs] = {};
    TextureHandle depthStencilTexture              = Undefined;

    uint16_t width  = 0;
    uint16_t height = 0;
    float    scaleX = 1.0f;
    float    scaleY = 1.0f;
    uint8_t  resize = 1;

    const char* name = nullptr;

    FramebufferCreation& addRenderTexture(TextureHandle texture) {
        outputTextures[numRenderTargets++] = texture;
        return *this;
    }

    FramebufferCreation& setDepthStencilTexture(TextureHandle texture) {
        depthStencilTexture = texture;
        return *this;
    }

    FramebufferCreation& setScaling(float scaleX, float scaleY, uint8_t resize) {
        this->scaleX = scaleX;
        this->scaleY = scaleY;
        this->resize = resize;
        return *this;
    }

    FramebufferCreation& setName(const char* name) {
        this->name = name;
        return *this;
    }
};

template <typename H>
struct Resource {
    H               handle;
    gerium_uint16_t references;

    gerium_uint16_t addReference() noexcept {
        return ++references;
    }

    gerium_uint16_t removeReference() {
        assert(references != 0);
        return --references;
    }
};

struct Texture : Resource<TextureHandle> {
    VkImage               vkImage;
    VkImageView           vkImageView;
    VkFormat              vkFormat;
    VkImageLayout         vkImageLayout;
    VmaAllocation         vmaAllocation;
    uint16_t              width;
    uint16_t              height;
    uint16_t              depth;
    uint8_t               mipmaps;
    TextureFlags          flags;
    gerium_texture_type_t type;
    gerium_utf8_t         name;

    //SamplerHandle sampler;
};

struct RenderPass : Resource<RenderPassHandle> {
    VkRenderPass     vkRenderPass;
    RenderPassOutput output;
    gerium_utf8_t    name;
};

struct Framebuffer : Resource<FramebufferHandle> {
    VkFramebuffer    vkFramebuffer;
    RenderPassHandle renderPass;
    uint16_t         width;
    uint16_t         height;
    float            scaleX;
    float            scaleY;

    gerium_uint32_t numColorAttachments;
    TextureHandle   colorAttachments[kMaxImageOutputs];
    TextureHandle   depthStencilAttachment;

    uint8_t resize;

    gerium_utf8_t name;
};

// clang-format on

} // namespace gerium::vulkan

#endif
