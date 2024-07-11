#include "Gerium.hpp"

#ifndef GERIUM_MIMALLOC_DISABLE
# include <mimalloc-new-delete.h>
#endif

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

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

        case GERIUM_RESULT_ERROR_FROM_CALLBACK:
            return "error from callback";

        case GERIUM_RESULT_ERROR_FEATURE_NOT_SUPPORTED:
            return "feature not supported";

        case GERIUM_RESULT_ERROR_INVALID_ARGUMENT:
            return "invalid argument passed";

        case GERIUM_RESULT_ERROR_NO_DISPLAY:
            return "no display";

        case GERIUM_RESULT_ERROR_APPLICATION_ALREADY_RUNNING:
            return "application already running";

        case GERIUM_RESULT_ERROR_APPLICATION_NOT_RUNNING:
            return "application not running";

        case GERIUM_RESULT_ERROR_APPLICATION_TERMINATED:
            return "application terminated";

        case GERIUM_RESULT_ERROR_CHANGE_DISPLAY_MODE:
            return "failed to change display mode";

        default:
            return "<unknown result>";
    }
}
