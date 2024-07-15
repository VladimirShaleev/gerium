#ifndef GERIUM_HANDLES_HPP
#define GERIUM_HANDLES_HPP

#include "Gerium.hpp"
#include "ResourcePool.hpp"

namespace gerium {

struct BufferHandle : Handle {};

struct TextureHandle : Handle {};

struct RenderPassHandle : Handle {};

struct FramebufferHandle : Handle {};

} // namespace gerium

#endif
