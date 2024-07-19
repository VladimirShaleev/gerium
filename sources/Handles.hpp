#ifndef GERIUM_HANDLES_HPP
#define GERIUM_HANDLES_HPP

#include "Gerium.hpp"
#include "ResourcePool.hpp"

namespace gerium {

struct BufferHandle : Handle {};

struct TextureHandle : Handle {};

struct MaterialHandle : Handle {};

struct DescriptorSetHandle : Handle {};

struct RenderPassHandle : Handle {};

struct FramebufferHandle : Handle {};

enum class ResourceUsageType {
    Immutable,
    Dynamic,
    Staging
};

enum class BufferUsageFlags {
    Vertex  = 1,
    Index   = 2,
    Uniform = 4
};
GERIUM_FLAGS(BufferUsageFlags)

enum class TextureFlags {
    None         = 0,
    RenderTarget = 1,
    Compute      = 2
};
GERIUM_FLAGS(TextureFlags)

struct BufferCreation {
    BufferUsageFlags usageFlags = {};
    ResourceUsageType usage     = ResourceUsageType::Immutable;
    uint32_t size               = 0;
    bool persistent             = false;
    void* initialData           = nullptr;
    const char* name            = nullptr;

    BufferCreation& reset() {
        usageFlags  = {};
        usage       = ResourceUsageType::Immutable;
        size        = 0;
        persistent  = false;
        initialData = nullptr;
        name        = nullptr;
        return *this;
    }

    BufferCreation& set(BufferUsageFlags flags, ResourceUsageType usage, uint32_t size) noexcept {
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

} // namespace gerium

#endif
