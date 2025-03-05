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

enum class ResourceUsageType : gerium_uint8_t {
    Immutable,
    Dynamic,
    Staging
};

enum class TextureFlags : gerium_uint8_t {
    None         = 0,
    RenderTarget = 1,
    Compute      = 2
};
GERIUM_FLAGS(TextureFlags)

enum class TextureCompressionFlags : gerium_uint8_t {
    None     = 0,
    ETC2     = 1,
    ASTC_LDR = 2,
    BC       = 4
};
GERIUM_FLAGS(TextureCompressionFlags)

enum class RenderPassOp : gerium_uint8_t {
    DontCare = 0,
    Load     = 1,
    Clear    = 2,
};

struct BufferCreation {
    gerium_buffer_usage_flags_t usageFlags = {};
    ResourceUsageType usage                = ResourceUsageType::Immutable;
    gerium_uint32_t size                   = 0;
    bool persistent                        = false;
    bool hasFillValue                      = false;
    gerium_data_t initialData              = nullptr;
    gerium_uint32_t fillValue              = 0;
    gerium_utf8_t name                     = nullptr;

    BufferCreation& set(gerium_buffer_usage_flags_t flags, ResourceUsageType usage, gerium_uint32_t size) noexcept {
        this->usageFlags = flags;
        this->usage      = usage;
        this->size       = size;
        return *this;
    }

    BufferCreation& setInitialData(gerium_data_t data) noexcept {
        initialData = data;
        return *this;
    }

    BufferCreation& setFillValue(gerium_uint32_t fillValue) noexcept {
        assert(!initialData);
        hasFillValue    = true;
        this->fillValue = fillValue;
        return *this;
    }

    BufferCreation& setName(gerium_utf8_t name) noexcept {
        this->name = name;
        return *this;
    }

    BufferCreation& setPersistent(bool value) noexcept {
        persistent = value;
        return *this;
    }
};

struct TextureCreation {
    gerium_uint16_t width      = 1;
    gerium_uint16_t height     = 1;
    gerium_uint16_t depth      = 1;
    gerium_uint8_t mipmaps     = 1;
    gerium_uint8_t layers      = 1;
    TextureFlags flags         = TextureFlags::None;
    gerium_format_t format     = GERIUM_FORMAT_R8G8B8A8_UNORM;
    gerium_texture_type_t type = GERIUM_TEXTURE_TYPE_2D;
    TextureHandle alias        = Undefined;
    gerium_data_t initialData  = nullptr;
    gerium_utf8_t name         = nullptr;

    TextureCreation& setSize(gerium_uint16_t width, gerium_uint16_t height, gerium_uint16_t depth) noexcept {
        this->width  = width;
        this->height = height;
        this->depth  = depth;
        return *this;
    }

    TextureCreation& setFlags(gerium_uint8_t mipmaps, gerium_uint8_t layers, bool renderTarget, bool compute) noexcept {
        this->mipmaps = mipmaps;
        this->layers  = layers;
        this->flags |= renderTarget ? TextureFlags::RenderTarget : TextureFlags::None;
        this->flags |= compute ? TextureFlags::Compute : TextureFlags::None;
        return *this;
    }

    TextureCreation& setFormat(gerium_format_t format, gerium_texture_type_t type) noexcept {
        this->format = format;
        this->type   = type;
        return *this;
    }

    TextureCreation& setAlias(TextureHandle alias) noexcept {
        this->alias = alias;
        return *this;
    }

    TextureCreation& setData(gerium_data_t data) noexcept {
        this->initialData = data;
        return *this;
    }

    TextureCreation& setName(gerium_utf8_t name) noexcept {
        this->name = name;
        return *this;
    }

    TextureCreation& build() noexcept {
        const auto maxMipLevels = gerium_uint8_t(calcMipLevels(width, height));

        width  = std::max(width, gerium_uint16_t(1));
        height = std::max(height, gerium_uint16_t(1));
        depth  = std::max(depth, gerium_uint16_t(1));
        layers = std::max(layers, gerium_uint8_t(type == GERIUM_TEXTURE_TYPE_CUBE ? 6 : 1));
        if (mipmaps == 0) {
            mipmaps = maxMipLevels;
        } else if (mipmaps != 1) {
            mipmaps = std::clamp(mipmaps, gerium_uint8_t(1), maxMipLevels);
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
    gerium_utf8_t name              = nullptr;

    TextureViewCreation& setTexture(TextureHandle texture) noexcept {
        this->texture = texture;
        return *this;
    }

    TextureViewCreation& setType(gerium_texture_type_t type) noexcept {
        this->type = type;
        return *this;
    }

    TextureViewCreation& setMips(gerium_uint16_t mipBaseLevel, gerium_uint16_t mipLevelCount) noexcept {
        this->mipBaseLevel  = mipBaseLevel;
        this->mipLevelCount = mipLevelCount;
        return *this;
    }

    TextureViewCreation& setArray(gerium_uint16_t arrayBaseLayer, gerium_uint16_t arrayLayerCount) noexcept {
        this->arrayBaseLayer  = arrayBaseLayer;
        this->arrayLayerCount = arrayLayerCount;
        return *this;
    }

    TextureViewCreation& setName(gerium_utf8_t name) noexcept {
        this->name = name;
        return *this;
    }
};

} // namespace gerium

#endif
