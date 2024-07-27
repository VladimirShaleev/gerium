#ifndef GERIUM_WINDOWS_VULKAN_ENUMS_HPP
#define GERIUM_WINDOWS_VULKAN_ENUMS_HPP

#include "../Gerium.hpp"

namespace gerium::vulkan {

enum class ResourceState {
    Undefined               = 0,
    VertexAndConstantBuffer = 0x1,
    IndexBuffer             = 0x2,
    RenderTarget            = 0x4,
    UnorderedAccess         = 0x8,
    DepthWrite              = 0x10,
    DepthRead               = 0x20,
    NonPixelShaderResource  = 0x40,
    PixelShaderResource     = 0x80,
    ShaderResource          = NonPixelShaderResource | PixelShaderResource,
    StreamOut               = 0x100,
    IndirectArgument        = 0x200,
    CopyDest                = 0x400,
    CopySource              = 0x800,
    GenericRead             = VertexAndConstantBuffer | IndexBuffer | ShaderResource | IndirectArgument | CopySource,
    Present                 = 0x1000,
    Common                  = 0x2000,
    RaytracingAccelerationStructure = 0x4000,
    ShadingRateSource               = 0x8000
};
GERIUM_FLAGS(ResourceState)

enum class QueueType {
    Graphics,
    Compute,
    CopyTransfer
};

} // namespace gerium::vulkan

#endif
