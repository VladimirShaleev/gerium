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

enum class ResourceState {
    Undefined                       = 0,
    VertexAndConstantBuffer         = 0x1,
    IndexBuffer                     = 0x2,
    RenderTarget                    = 0x4,
    UnorderedAccess                 = 0x8,
    DepthWrite                      = 0x10,
    DepthRead                       = 0x20,
    NonPixelShaderResource          = 0x40,
    PixelShaderResource             = 0x80,
    ShaderResource                  = NonPixelShaderResource | 0x80,
    StreamOut                       = 0x100,
    IndirectArgument                = 0x200,
    CopyDest                        = 0x400,
    CopySource                      = 0x800,
    GenericRead                     = VertexAndConstantBuffer | IndexBuffer | ShaderResource | IndirectArgument | CopySource,
    Present                         = 0x1000,
    Common                          = 0x2000,
    RaytracingAccelerationStructure = 0x4000,
    ShadingRateSource               = 0x8000
};
GERIUM_FLAGS(ResourceState)

enum class QueueType {
    Graphics,
    Compute,
    CopyTransfer
};

enum class FillMode {
    Wireframe,
    Solid,
    Point
};

enum class ColorWriteEnabled {
    Red   = 0,
    Green = 1,
    Blue  = 2,
    Alpha = 4,
    All   = Red | Green | Blue | Alpha
};
GERIUM_FLAGS(ColorWriteEnabled)

enum class VertexInputRate {
    PerVertex,
    PerInstance
};

enum class VertexComponentFormat {
    Float,
    Float2,
    Float3,
    Float4,
    Mat4,
    Byte,
    Byte4N,
    UByte,
    UByte4N,
    Short2,
    Short2N,
    Short4,
    Short4N,
    Uint,
    Uint2,
    Uint4
};

} // namespace gerium::vulkan

#endif
