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

enum class ResourceUsageType : uint8_t {
    Immutable,
    Dynamic,
    Staging
};

enum class TextureFlags : uint8_t {
    None         = 0,
    RenderTarget = 1,
    Compute      = 2
};
GERIUM_FLAGS(TextureFlags)

enum class RenderPassOp : uint8_t {
    DontCare = 0,
    Load     = 1,
    Clear    = 2,
};

struct BufferCreation {
    gerium_buffer_usage_flags_t usageFlags = {};
    ResourceUsageType usage                = ResourceUsageType::Immutable;
    uint32_t size                          = 0;
    bool persistent                        = false;
    void* initialData                      = nullptr;
    bool hasFillValue                      = false;
    gerium_uint32_t fillValue              = 0;
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

    BufferCreation& setFillValue(gerium_uint32_t fillValue) noexcept {
        assert(!initialData);
        hasFillValue    = true;
        this->fillValue = fillValue;
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
    uint8_t  mipmaps           = 1;
    uint8_t  layers            = 1;
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

    TextureCreation& setFlags(uint8_t mipmaps, uint8_t layers, bool renderTarget, bool compute) {
        this->mipmaps = mipmaps;
        this->layers  = layers;
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

    TextureCreation& build() {
        const auto maxMipLevels = uint8_t(calcMipLevels(width, height));

        width  = std::max(width, uint16_t(1));
        height = std::max(height, uint16_t(1));
        depth  = std::max(depth, uint16_t(1));
        layers = std::max(layers, uint8_t(type == GERIUM_TEXTURE_TYPE_CUBE ? 6 : 1));
        if (mipmaps == 0) {
            mipmaps = maxMipLevels;
        } else if (mipmaps != 1) {
            mipmaps = std::clamp(mipmaps, uint8_t(1), maxMipLevels);
        }
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
