#include "Utils.hpp"
#include "../Exceptions.hpp"

namespace gerium::vulkan {

void check(VkResult result) {
    switch (result) {
        case VK_SUCCESS:
        case VK_NOT_READY:
        case VK_TIMEOUT:
        case VK_EVENT_SET:
        case VK_EVENT_RESET:
        case VK_INCOMPLETE:
        case VK_SUBOPTIMAL_KHR:
        case VK_THREAD_IDLE_KHR:
        case VK_THREAD_DONE_KHR:
        case VK_OPERATION_DEFERRED_KHR:
        case VK_OPERATION_NOT_DEFERRED_KHR:
        case VK_PIPELINE_COMPILE_REQUIRED:
        case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            throw Exception(GERIUM_RESULT_ERROR_OUT_OF_MEMORY);
        case VK_ERROR_INITIALIZATION_FAILED:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_DEVICE_LOST:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_MEMORY_MAP_FAILED:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_LAYER_NOT_PRESENT:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_FEATURE_NOT_PRESENT:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_TOO_MANY_OBJECTS:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_FRAGMENTED_POOL:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_UNKNOWN:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN);
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_FRAGMENTATION:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_SURFACE_LOST_KHR:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_OUT_OF_DATE_KHR:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_VALIDATION_FAILED_EXT:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_INVALID_SHADER_NV:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_NOT_PERMITTED_KHR:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
            throw Exception(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error
        default:
            assert(!"unreachable code");
    }
}

static void* VKAPI_PTR allocation(void* pUserData,
                                  size_t size,
                                  size_t alignment,
                                  VkSystemAllocationScope allocationScope) {
    return mi_realloc_aligned(nullptr, size, alignment);
}

static void* VKAPI_PTR
reallocation(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
    return mi_realloc_aligned(pOriginal, size, alignment);
}

static void VKAPI_PTR free(void* pUserData, void* pMemory) {
    mi_free(pMemory);
}

static void VKAPI_PTR internalAllocationNotification(void* pUserData,
                                                     size_t size,
                                                     VkInternalAllocationType allocationType,
                                                     VkSystemAllocationScope allocationScope) {
}

static void VKAPI_PTR internalFreeNotification(void* pUserData,
                                               size_t size,
                                               VkInternalAllocationType allocationType,
                                               VkSystemAllocationScope allocationScope) {
}

const VkAllocationCallbacks* getAllocCalls() noexcept {
    static constexpr VkAllocationCallbacks callbacks{
        nullptr, allocation, reallocation, free, internalAllocationNotification, internalFreeNotification
    };
    return &callbacks;
}

} // namespace gerium::vulkan
