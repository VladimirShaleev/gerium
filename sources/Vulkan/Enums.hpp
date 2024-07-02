#include "../Gerium.hpp"

enum class TextureFlags {
    None         = 0,
    RenderTarget = 1,
    Compute      = 2
};
GERIUM_FLAGS(TextureFlags)
