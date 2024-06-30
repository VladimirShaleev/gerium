#ifndef GERIUM_WINDOWS_VULKAN_UTILS_HPP
#define GERIUM_WINDOWS_VULKAN_UTILS_HPP

#include "../Gerium.hpp"

namespace gerium::vulkan {

void check(VkResult result);

const VkAllocationCallbacks* getAllocCalls() noexcept;

} // namespace gerium::vulkan

#endif
