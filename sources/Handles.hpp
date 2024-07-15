#ifndef GERIUM_HANDLES_HPP
#define GERIUM_HANDLES_HPP

#include "Gerium.hpp"
#include "ResourcePool.hpp"

namespace gerium {

struct BufferHandle : Handle {};

struct TextureHandle : Handle {};

struct RenderPassHandle : Handle {};

struct FramebufferHandle : Handle {};

enum class TextureFlags {
    None         = 0,
    RenderTarget = 1,
    Compute      = 2
};
GERIUM_FLAGS(TextureFlags)

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

} // namespace gerium

#endif
