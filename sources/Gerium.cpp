#include "Gerium.hpp"

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

        case GERIUM_RESULT_UNKNOWN_ERROR:
            return "out of memory";

        case GERIUM_RESULT_OUT_OF_MEMORY:
            return "unknown error";

        case GERIUM_RESULT_NOT_IMPLEMENTED:
            return "not implemented";

        case GERIUM_RESULT_FEATURE_NOT_SUPPORTED:
            return "feature not supported";

        case GERIUM_RESULT_INVALID_ARGUMENT:
            return "invalid argument passed";

        case GERIUM_RESULT_NO_DISPLAY:
            return "no display";

        case GERIUM_RESULT_APPLICATION_RUNNING:
            return "application running";

        case GERIUM_RESULT_APPLICATION_TERMINATED:
            return "application terminated";

        default:
            return "<unknown result>";
    }
}
