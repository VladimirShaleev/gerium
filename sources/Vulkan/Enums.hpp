#ifndef GERIUM_WINDOWS_VULKAN_ENUMS_HPP
#define GERIUM_WINDOWS_VULKAN_ENUMS_HPP

#include "../Gerium.hpp"

namespace gerium::vulkan {

enum class TextureFlags {
    None         = 0,
    RenderTarget = 1,
    Compute      = 2
};
GERIUM_FLAGS(TextureFlags)

enum class RenderPassOperation {
    DontCare,
    Load,
    Clear,
    Count
};

} // namespace gerium::vulkan

#endif
