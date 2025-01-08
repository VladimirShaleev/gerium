#include "Gerium.hpp"

#ifndef GERIUM_MIMALLOC_DISABLE
# include <mimalloc-new-delete.h>
#endif

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace gerium {

gerium_uint32_t nameColor(gerium_utf8_t name) {
    constexpr uint32_t colors[] = { 0xFF010067, 0xFF00FF00, 0xFFFF0000, 0xFF0000FF, 0xFFFEFF01, 0xFFFEA6FF, 0xFF66DBFF,
                                    0xFF016400, 0xFF670001, 0xFF3A0095, 0xFFB57D00, 0xFFF600FF, 0xFFE8EEFF, 0xFF004D77,
                                    0xFF92FB90, 0xFFFF7600, 0xFF00FFD5, 0xFF7E93FF, 0xFF6C826A, 0xFF9D02FF, 0xFF0089FE,
                                    0xFF82477A, 0xFFD22D7E, 0xFF00A985, 0xFF5600FF, 0xFF0024A4, 0xFF7EAE00, 0xFF3B3D68,
                                    0xFFFFC6BD, 0xFF003426, 0xFF93D3BD, 0xFF17B900, 0xFF8E009E, 0xFF441500, 0xFF9F8CC2,
                                    0xFFA374FF, 0xFFFFD001, 0xFF544700, 0xFFFE6FE5, 0xFF318278, 0xFFA14C0E, 0xFFCBD091,
                                    0xFF7099BE, 0xFFE88A96, 0xFF0088BB, 0xFF2C0043, 0xFF74FFDE, 0xFFC6FF00, 0xFF02E5FF,
                                    0xFF000E62, 0xFF9C8F00, 0xFF52FF98, 0xFFB14475, 0xFFFF00B5, 0xFF78FF00, 0xFF416EFF,
                                    0xFF395F00, 0xFF82686B, 0xFF4EAD5F, 0xFF4057A7, 0xFFD2FFA5, 0xFF67B1FF, 0xFFFF9B00,
                                    0xFFBE5EE8 };

    static std::map<uint64_t, uint32_t> nameColors;

    const auto nameHash = hash(name);

    if (auto it = nameColors.find(nameHash); it != nameColors.end()) {
        return it->second;
    }

    const auto newColor  = colors[nameColors.size() % std::size(colors)];
    nameColors[nameHash] = newColor;
    return newColor;
}

glm::vec4 nameColorVec4(gerium_utf8_t name) {
    auto color = nameColor(name);
    return glm::vec4(float((color >> 0) & 0xFF) / 255.0f,
                     float((color >> 8) & 0xFF) / 255.0f,
                     float((color >> 16) & 0xFF) / 255.0f,
                     float((color >> 24) & 0xFF) / 255.0f);
}

} // namespace gerium

gerium_uint32_t gerium_version(void) {
    return GERIUM_VERSION;
}

gerium_utf8_t gerium_version_string(void) {
    return GERIUM_VERSION_STRING;
}

gerium_utf8_t gerium_result_to_string(gerium_result_t result) {
    switch (result) {
        case GERIUM_RESULT_SUCCESS:
            return "no error has occurred";

        case GERIUM_RESULT_SKIP_FRAME:
            return "frame skip request";

        case GERIUM_RESULT_ERROR_UNKNOWN:
            return "unknown error";

        case GERIUM_RESULT_ERROR_OUT_OF_MEMORY:
            return "out of memory";

        case GERIUM_RESULT_ERROR_NOT_IMPLEMENTED:
            return "not implemented";

        case GERIUM_RESULT_ERROR_FILE_OPEN:
            return "failed to open file";

        case GERIUM_RESULT_ERROR_FILE_ALLOCATE:
            return "failed to allocate file";

        case GERIUM_RESULT_ERROR_FILE_WRITE:
            return "error writing to file";

        case GERIUM_RESULT_ERROR_APPLICATION_CREATE:
            return "application create failed";

        case GERIUM_RESULT_ERROR_APPLICATION_ALREADY_RUNNING:
            return "application already running";

        case GERIUM_RESULT_ERROR_APPLICATION_NOT_RUNNING:
            return "application not running";

        case GERIUM_RESULT_ERROR_APPLICATION_TERMINATED:
            return "application terminated";

        case GERIUM_RESULT_ERROR_NO_DISPLAY:
            return "no display";

        case GERIUM_RESULT_ERROR_DEVICE_SELECTION:
            return "failed device selection";

        case GERIUM_RESULT_ERROR_DEVICE_LOST:
            return "device lost";

        case GERIUM_RESULT_ERROR_ALREADY_EXISTS:
            return "already exists";

        case GERIUM_RESULT_ERROR_NOT_FOUND:
            return "not found";

        case GERIUM_RESULT_ERROR_FROM_CALLBACK:
            return "error from callback";

        case GERIUM_RESULT_ERROR_FEATURE_NOT_SUPPORTED:
            return "feature not supported";

        case GERIUM_RESULT_ERROR_FORMAT_NOT_SUPPORTED:
            return "Format not supported";

        case GERIUM_RESULT_ERROR_FIDELITY_FX_NOT_SUPPORTED:
            return "FidelityFX not supported";

        case GERIUM_RESULT_ERROR_INVALID_ARGUMENT:
            return "invalid argument passed";

        case GERIUM_RESULT_ERROR_INVALID_FRAME_GRAPH:
            return "invalid frame graph";

        case GERIUM_RESULT_ERROR_INVALID_RESOURCE:
            return "invalid resource";

        case GERIUM_RESULT_ERROR_INVALID_OPERATION:
            return "invalid operation";

        case GERIUM_RESULT_ERROR_PARSE_SPIRV:
            return "error parse spirv";

        case GERIUM_RESULT_ERROR_DETECT_SHADER_LANGUAGE:
            return "detect shader language failed";

        case GERIUM_RESULT_ERROR_COMPILE_SHADER:
            return "compile shader failed";

        case GERIUM_RESULT_ERROR_BINDING:
            return "error binding";

        case GERIUM_RESULT_ERROR_DESCRIPTOR:
            return "error descriptor";

        case GERIUM_RESULT_ERROR_LOAD_TEXTURE:
            return "error load texture";

        case GERIUM_RESULT_ERROR_CHANGE_DISPLAY_MODE:
            return "failed to change display mode";

        default:
            return "<unknown result>";
    }
}
