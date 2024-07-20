#ifndef GERIUM_WINDOWS_VULKAN_UTILS_HPP
#define GERIUM_WINDOWS_VULKAN_UTILS_HPP

#include "../Gerium.hpp"
#include "../Handles.hpp"
#include "Enums.hpp"

namespace gerium::vulkan {

void check(VkResult result);

const VkAllocationCallbacks* getAllocCalls() noexcept;

gerium_inline bool hasDepthOrStencil(VkFormat format) noexcept {
    return format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D24_UNORM_S8_UINT;
}

gerium_inline bool hasStencil(VkFormat format) noexcept {
    return format >= VK_FORMAT_S8_UINT && format <= VK_FORMAT_D32_SFLOAT_S8_UINT;
}

gerium_inline bool hasDepth(VkFormat format) noexcept {
    return (format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D32_SFLOAT) ||
           (format >= VK_FORMAT_D16_UNORM_S8_UINT && format <= VK_FORMAT_D32_SFLOAT_S8_UINT);
}

gerium_inline VkFormat toVkFormat(gerium_format_t format) noexcept {
    switch (format) {
        case GERIUM_FORMAT_R8_UNORM:
            return VK_FORMAT_R8_UNORM;
        case GERIUM_FORMAT_R8_SNORM:
            return VK_FORMAT_R8_SNORM;
        case GERIUM_FORMAT_R8_UINT:
            return VK_FORMAT_R8_UINT;
        case GERIUM_FORMAT_R8_SINT:
            return VK_FORMAT_R8_SINT;
        case GERIUM_FORMAT_R8G8_UNORM:
            return VK_FORMAT_R8G8_UNORM;
        case GERIUM_FORMAT_R8G8_SNORM:
            return VK_FORMAT_R8G8_SNORM;
        case GERIUM_FORMAT_R8G8_UINT:
            return VK_FORMAT_R8G8_UINT;
        case GERIUM_FORMAT_R8G8_SINT:
            return VK_FORMAT_R8G8_SINT;
        case GERIUM_FORMAT_R8G8B8_UNORM:
            return VK_FORMAT_R8G8B8_UNORM;
        case GERIUM_FORMAT_R8G8B8_SNORM:
            return VK_FORMAT_R8G8B8_SNORM;
        case GERIUM_FORMAT_R8G8B8_UINT:
            return VK_FORMAT_R8G8B8_UINT;
        case GERIUM_FORMAT_R8G8B8_SINT:
            return VK_FORMAT_R8G8B8_SINT;
        case GERIUM_FORMAT_R8G8B8_SRGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case GERIUM_FORMAT_R4G4B4A4_UNORM:
            return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        case GERIUM_FORMAT_R5G5B5A1_UNORM:
            return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
        case GERIUM_FORMAT_R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case GERIUM_FORMAT_R8G8B8A8_SNORM:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case GERIUM_FORMAT_R8G8B8A8_UINT:
            return VK_FORMAT_R8G8B8A8_UINT;
        case GERIUM_FORMAT_R8G8B8A8_SINT:
            return VK_FORMAT_R8G8B8A8_SINT;
        case GERIUM_FORMAT_R8G8B8A8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case GERIUM_FORMAT_A2R10G10B10_UNORM:
            return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        case GERIUM_FORMAT_A2R10G10B10_UINT:
            return VK_FORMAT_A2R10G10B10_UINT_PACK32;
        case GERIUM_FORMAT_R16_UINT:
            return VK_FORMAT_R16_UINT;
        case GERIUM_FORMAT_R16_SINT:
            return VK_FORMAT_R16_SINT;
        case GERIUM_FORMAT_R16_SFLOAT:
            return VK_FORMAT_R16_SFLOAT;
        case GERIUM_FORMAT_R16G16_UINT:
            return VK_FORMAT_R16G16_UINT;
        case GERIUM_FORMAT_R16G16_SINT:
            return VK_FORMAT_R16G16_SINT;
        case GERIUM_FORMAT_R16G16_SFLOAT:
            return VK_FORMAT_R16G16_SFLOAT;
        case GERIUM_FORMAT_R16G16B16_UINT:
            return VK_FORMAT_R16G16B16_UINT;
        case GERIUM_FORMAT_R16G16B16_SINT:
            return VK_FORMAT_R16G16B16_SINT;
        case GERIUM_FORMAT_R16G16B16_SFLOAT:
            return VK_FORMAT_R16G16B16_SFLOAT;
        case GERIUM_FORMAT_R16G16B16A16_UINT:
            return VK_FORMAT_R16G16B16A16_UINT;
        case GERIUM_FORMAT_R16G16B16A16_SINT:
            return VK_FORMAT_R16G16B16A16_SINT;
        case GERIUM_FORMAT_R16G16B16A16_SFLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case GERIUM_FORMAT_R32_UINT:
            return VK_FORMAT_R32_UINT;
        case GERIUM_FORMAT_R32_SINT:
            return VK_FORMAT_R32_SINT;
        case GERIUM_FORMAT_R32_SFLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case GERIUM_FORMAT_R32G32_UINT:
            return VK_FORMAT_R32G32_UINT;
        case GERIUM_FORMAT_R32G32_SINT:
            return VK_FORMAT_R32G32_SINT;
        case GERIUM_FORMAT_R32G32_SFLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case GERIUM_FORMAT_R32G32B32_UINT:
            return VK_FORMAT_R32G32B32_UINT;
        case GERIUM_FORMAT_R32G32B32_SINT:
            return VK_FORMAT_R32G32B32_SINT;
        case GERIUM_FORMAT_R32G32B32_SFLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case GERIUM_FORMAT_R32G32B32A32_UINT:
            return VK_FORMAT_R32G32B32A32_UINT;
        case GERIUM_FORMAT_R32G32B32A32_SINT:
            return VK_FORMAT_R32G32B32A32_SINT;
        case GERIUM_FORMAT_R32G32B32A32_SFLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case GERIUM_FORMAT_B10G11R11_UFLOAT:
            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case GERIUM_FORMAT_E5B9G9R9_UFLOAT:
            return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case GERIUM_FORMAT_D16_UNORM:
            return VK_FORMAT_D16_UNORM;
        case GERIUM_FORMAT_X8_D24_UNORM:
            return VK_FORMAT_X8_D24_UNORM_PACK32;
        case GERIUM_FORMAT_D32_SFLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case GERIUM_FORMAT_S8_UINT:
            return VK_FORMAT_S8_UINT;
        case GERIUM_FORMAT_D24_UNORM_S8_UINT:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case GERIUM_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        default:
            assert(!"unreachable code");
            return VK_FORMAT_UNDEFINED;
    }
}

gerium_inline VkPolygonMode toVkPolygonMode(gerium_polygon_mode_t polygonMode) noexcept {
    constexpr VkPolygonMode modes[] = { VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT };
    return modes[int(polygonMode)];
}

gerium_inline VkCullModeFlags toVkCullMode(gerium_cull_mode_t cullMode) noexcept {
    constexpr VkCullModeFlags modes[] = {
        VK_CULL_MODE_NONE, VK_CULL_MODE_FRONT_BIT, VK_CULL_MODE_BACK_BIT, VK_CULL_MODE_FRONT_AND_BACK
    };
    return modes[int(cullMode)];
}

gerium_inline VkCompareOp toVkCompareOp(gerium_compare_op_t compareOp) noexcept {
    constexpr VkCompareOp compares[] = { VK_COMPARE_OP_NEVER,   VK_COMPARE_OP_ALWAYS,
                                         VK_COMPARE_OP_LESS,    VK_COMPARE_OP_LESS_OR_EQUAL,
                                         VK_COMPARE_OP_GREATER, VK_COMPARE_OP_GREATER_OR_EQUAL,
                                         VK_COMPARE_OP_EQUAL,   VK_COMPARE_OP_NOT_EQUAL };
    return compares[int(compareOp)];
}

gerium_inline VkStencilOp toVkStencilOp(gerium_stencil_op_t stencilOp) noexcept {
    constexpr VkStencilOp stencils[] = { VK_STENCIL_OP_KEEP,
                                         VK_STENCIL_OP_ZERO,
                                         VK_STENCIL_OP_REPLACE,
                                         VK_STENCIL_OP_INVERT,
                                         VK_STENCIL_OP_INCREMENT_AND_CLAMP,
                                         VK_STENCIL_OP_DECREMENT_AND_CLAMP,
                                         VK_STENCIL_OP_INCREMENT_AND_WRAP,
                                         VK_STENCIL_OP_DECREMENT_AND_WRAP };
    return stencils[int(stencilOp)];
}

gerium_inline VkLogicOp toVkLogicOp(gerium_logic_op_t logicOp) noexcept {
    constexpr VkLogicOp logics[] = { VK_LOGIC_OP_CLEAR,       VK_LOGIC_OP_SET,           VK_LOGIC_OP_NO_OP,
                                     VK_LOGIC_OP_COPY,        VK_LOGIC_OP_COPY_INVERTED, VK_LOGIC_OP_AND,
                                     VK_LOGIC_OP_AND_REVERSE, VK_LOGIC_OP_AND_INVERTED,  VK_LOGIC_OP_NAND,
                                     VK_LOGIC_OP_OR,          VK_LOGIC_OP_OR_REVERSE,    VK_LOGIC_OP_OR_INVERTED,
                                     VK_LOGIC_OP_NOR,         VK_LOGIC_OP_XOR,           VK_LOGIC_OP_EQUIVALENT,
                                     VK_LOGIC_OP_INVERT };
    return logics[int(logicOp)];
}

gerium_inline VkBlendOp toVkBlendOp(gerium_blend_op_t blendOp) noexcept {
    constexpr VkBlendOp blends[] = {
        VK_BLEND_OP_ADD, VK_BLEND_OP_SUBTRACT, VK_BLEND_OP_REVERSE_SUBTRACT, VK_BLEND_OP_MIN, VK_BLEND_OP_MAX
    };
    return blends[int(blendOp)];
}

gerium_inline VkBlendFactor toVkBlendFactor(gerium_blend_factor_t blendFactor) noexcept {
    constexpr VkBlendFactor factors[] = { VK_BLEND_FACTOR_ZERO,
                                          VK_BLEND_FACTOR_ONE,
                                          VK_BLEND_FACTOR_SRC_COLOR,
                                          VK_BLEND_FACTOR_SRC_ALPHA,
                                          VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
                                          VK_BLEND_FACTOR_DST_COLOR,
                                          VK_BLEND_FACTOR_DST_ALPHA,
                                          VK_BLEND_FACTOR_CONSTANT_COLOR,
                                          VK_BLEND_FACTOR_CONSTANT_ALPHA,
                                          VK_BLEND_FACTOR_SRC1_COLOR,
                                          VK_BLEND_FACTOR_SRC1_ALPHA,
                                          VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
                                          VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                          VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
                                          VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
                                          VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
                                          VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
                                          VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
                                          VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA };
    return factors[int(blendFactor)];
}

gerium_inline VkColorComponentFlags toVkColorComponent(gerium_color_component_flags_t colorComponentFlags) noexcept {
    return (VkColorComponentFlags) colorComponentFlags;
}

gerium_inline VkFrontFace toVkFrontFace(gerium_front_face_t frontFace) noexcept {
    constexpr VkFrontFace faces[] = { VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FRONT_FACE_CLOCKWISE };
    return faces[int(frontFace)];
}

gerium_inline VkBufferUsageFlags toVkBufferUsageFlags(gerium_buffer_usage_flags_t flags) noexcept {
    VkBufferUsageFlags result{};

    if ((flags & GERIUM_BUFFER_USAGE_VERTEX) == GERIUM_BUFFER_USAGE_VERTEX) {
        result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    if ((flags & GERIUM_BUFFER_USAGE_INDEX) == GERIUM_BUFFER_USAGE_INDEX) {
        result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }

    if ((flags & GERIUM_BUFFER_USAGE_UNIFORM) == GERIUM_BUFFER_USAGE_UNIFORM) {
        result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    return result;
}

gerium_inline VkImageType toVkImageType(gerium_texture_type_t type) noexcept {
    switch (type) {
        case GERIUM_TEXTURE_TYPE_1D:
            return VK_IMAGE_TYPE_1D;
        case GERIUM_TEXTURE_TYPE_2D:
            return VK_IMAGE_TYPE_2D;
        case GERIUM_TEXTURE_TYPE_3D:
            return VK_IMAGE_TYPE_3D;
        case GERIUM_TEXTURE_TYPE_1D_ARRAY:
            return VK_IMAGE_TYPE_1D;
        case GERIUM_TEXTURE_TYPE_2D_ARRAY:
            return VK_IMAGE_TYPE_2D;
        case GERIUM_TEXTURE_TYPE_CUBE_ARRAY:
            return VK_IMAGE_TYPE_3D;
        default:
            assert(!"unreachable code");
            return {};
    }
}

gerium_inline VkImageViewType toVkImageViewType(gerium_texture_type_t type) noexcept {
    constexpr VkImageViewType types[] = { VK_IMAGE_VIEW_TYPE_1D,       VK_IMAGE_VIEW_TYPE_2D,
                                          VK_IMAGE_VIEW_TYPE_3D,       VK_IMAGE_VIEW_TYPE_1D_ARRAY,
                                          VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY };
    return types[(int) type];
}

gerium_inline VkFormat toVkVertexFormat(VertexComponentFormat value) {
    constexpr VkFormat sVkVertexFormats[] = { VK_FORMAT_R32_SFLOAT,
                                              VK_FORMAT_R32G32_SFLOAT,
                                              VK_FORMAT_R32G32B32_SFLOAT,
                                              VK_FORMAT_R32G32B32A32_SFLOAT,
                                              /*MAT4 TODO*/ VK_FORMAT_R32G32B32A32_SFLOAT,
                                              VK_FORMAT_R8_SINT,
                                              VK_FORMAT_R8G8B8A8_SNORM,
                                              VK_FORMAT_R8_UINT,
                                              VK_FORMAT_R8G8B8A8_UINT,
                                              VK_FORMAT_R16G16_SINT,
                                              VK_FORMAT_R16G16_SNORM,
                                              VK_FORMAT_R16G16B16A16_SINT,
                                              VK_FORMAT_R16G16B16A16_SNORM,
                                              VK_FORMAT_R32_UINT,
                                              VK_FORMAT_R32G32_UINT,
                                              VK_FORMAT_R32G32B32A32_UINT };

    return sVkVertexFormats[(int) value];
}

gerium_inline VkAccessFlags toVkAccessFlags(ResourceState state) noexcept {
    VkAccessFlags ret = 0;
    if ((state & ResourceState::CopySource) == ResourceState::CopySource) {
        ret |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if ((state & ResourceState::CopyDest) == ResourceState::CopyDest) {
        ret |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    if ((state & ResourceState::VertexAndConstantBuffer) == ResourceState::VertexAndConstantBuffer) {
        ret |= VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    }
    if ((state & ResourceState::IndexBuffer) == ResourceState::IndexBuffer) {
        ret |= VK_ACCESS_INDEX_READ_BIT;
    }
    if ((state & ResourceState::UnorderedAccess) == ResourceState::UnorderedAccess) {
        ret |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    }
    if ((state & ResourceState::IndirectArgument) == ResourceState::IndirectArgument) {
        ret |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    }
    if ((state & ResourceState::RenderTarget) == ResourceState::RenderTarget) {
        ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    if ((state & ResourceState::DepthWrite) == ResourceState::DepthWrite) {
        ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    }
    if ((state & ResourceState::ShaderResource) == ResourceState::ShaderResource) {
        ret |= VK_ACCESS_SHADER_READ_BIT;
    }
    if ((state & ResourceState::Present) == ResourceState::Present) {
        ret |= VK_ACCESS_MEMORY_READ_BIT;
    }
    return ret;
}

gerium_inline VkImageLayout toVkImageLayout(ResourceState usage) noexcept {
    if ((usage & ResourceState::CopySource) == ResourceState::CopySource) {
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }

    if ((usage & ResourceState::CopyDest) == ResourceState::CopyDest) {
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }

    if ((usage & ResourceState::RenderTarget) == ResourceState::RenderTarget) {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    if ((usage & ResourceState::DepthWrite) == ResourceState::DepthWrite) {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    if ((usage & ResourceState::DepthRead) == ResourceState::DepthRead) {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }

    if ((usage & ResourceState::UnorderedAccess) == ResourceState::UnorderedAccess) {
        return VK_IMAGE_LAYOUT_GENERAL;
    }

    if ((usage & ResourceState::ShaderResource) == ResourceState::ShaderResource) {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    if ((usage & ResourceState::Present) == ResourceState::Present) {
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    if (usage == ResourceState::Common) {
        return VK_IMAGE_LAYOUT_GENERAL;
    }

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

inline VkPipelineStageFlags utilDeterminePipelineStageFlags(VkAccessFlags accessFlags, QueueType queueType) noexcept {
    VkPipelineStageFlags flags = 0;

    switch (queueType) {
        case QueueType::Graphics: {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0) {
                flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
            }

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) !=
                0) {
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                /*if ( pRenderer->pActiveGpuSettings->mGeometryShaderSupported ) {
                    flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
                }
                if ( pRenderer->pActiveGpuSettings->mTessellationSupported ) {
                    flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
                    flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
                }*/
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }

            if ((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0) {
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }

            if ((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0) {
                flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            }

            if ((accessFlags &
                 (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0) {
                flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            }

            break;
        }
        case QueueType::Compute: {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0 ||
                (accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0 ||
                (accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0 ||
                (accessFlags &
                 (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0) {
                return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            }

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) !=
                0) {
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }

            break;
        }
        case QueueType::CopyTransfer:
            return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        default:
            break;
    }

    // Compatible with both compute and graphics queues
    if ((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0) {
        flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    }

    if ((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0) {
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    if ((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0) {
        flags |= VK_PIPELINE_STAGE_HOST_BIT;
    }

    if (flags == 0) {
        flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }

    return flags;
}

} // namespace gerium::vulkan

#endif
