#ifndef GERIUM_HANDLES_HPP
#define GERIUM_HANDLES_HPP

#include "File.hpp"
#include "Gerium.hpp"
#include "ResourcePool.hpp"

namespace gerium {

struct BufferHandle : Handle {};

struct TextureHandle : Handle {};

struct TechniqueHandle : Handle {};

struct DescriptorSetHandle : Handle {};

struct RenderPassHandle : Handle {};

struct FramebufferHandle : Handle {};

enum class ResourceUsageType {
    Immutable,
    Dynamic,
    Staging
};

enum class TextureFlags {
    None         = 0,
    RenderTarget = 1,
    Compute      = 2
};
GERIUM_FLAGS(TextureFlags)

struct BufferCreation {
    gerium_buffer_usage_flags_t usageFlags = {};
    ResourceUsageType usage                = ResourceUsageType::Immutable;
    uint32_t size                          = 0;
    bool persistent                        = false;
    void* initialData                      = nullptr;
    const char* name                       = nullptr;

    BufferCreation& reset() {
        usageFlags  = {};
        usage       = ResourceUsageType::Immutable;
        size        = 0;
        persistent  = false;
        initialData = nullptr;
        name        = nullptr;
        return *this;
    }

    BufferCreation& set(gerium_buffer_usage_flags_t flags, ResourceUsageType usage, uint32_t size) noexcept {
        this->usageFlags = flags;
        this->usage      = usage;
        this->size       = size;
        return *this;
    }

    BufferCreation& setInitialData(void* data) noexcept {
        initialData = data;
        return *this;
    }

    BufferCreation& setName(const char* name) noexcept {
        this->name = name;
        return *this;
    }

    BufferCreation& setPersistent(bool value) noexcept {
        persistent = value;
        return *this;
    }
};

struct TextureCreation {
    uint16_t width             = 1;
    uint16_t height            = 1;
    uint16_t depth             = 1;
    uint16_t mipmaps           = 1;
    TextureFlags flags         = TextureFlags::None;
    gerium_format_t format     = GERIUM_FORMAT_R8G8B8A8_UNORM;
    gerium_texture_type_t type = GERIUM_TEXTURE_TYPE_2D;
    TextureHandle alias        = Undefined;
    void* initialData          = nullptr;
    const char* name           = nullptr;

    TextureCreation& setSize(uint16_t width, uint16_t height, uint16_t depth) {
        this->width  = width;
        this->height = height;
        this->depth  = depth;
        return *this;
    }

    TextureCreation& setFlags(uint8_t mipmaps, bool renderTarget, bool compute) {
        this->mipmaps = mipmaps;
        this->flags |= renderTarget ? TextureFlags::RenderTarget : TextureFlags::None;
        this->flags |= compute ? TextureFlags::Compute : TextureFlags::None;
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

struct TextureViewCreation {
    TextureHandle texture           = Undefined;
    gerium_texture_type_t type      = GERIUM_TEXTURE_TYPE_2D;
    gerium_uint16_t mipBaseLevel    = 0;
    gerium_uint16_t mipLevelCount   = 1;
    gerium_uint16_t arrayBaseLayer  = 0;
    gerium_uint16_t arrayLayerCount = 1;
    const char* name                = nullptr;

    TextureViewCreation& setTexture(TextureHandle texture) {
        this->texture = texture;
        return *this;
    }

    TextureViewCreation& setType(gerium_texture_type_t type) {
        this->type = type;
        return *this;
    }

    TextureViewCreation& setMips(gerium_uint16_t mipBaseLevel, gerium_uint16_t mipLevelCount) {
        this->mipBaseLevel  = mipBaseLevel;
        this->mipLevelCount = mipLevelCount;
        return *this;
    }

    TextureViewCreation& setArray(gerium_uint16_t arrayBaseLayer, gerium_uint16_t arrayLayerCount) {
        this->arrayBaseLayer  = arrayBaseLayer;
        this->arrayLayerCount = arrayLayerCount;
        return *this;
    }

    TextureViewCreation& setName(const char* name) {
        this->name = name;
        return *this;
    }
};

} // namespace gerium

#endif
