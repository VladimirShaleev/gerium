#include "Gerium.hpp"

gerium_uint32_t gerium_version(void) {
    return GERIUM_VERSION;
}

gerium_utf8_t gerium_version_string(void) {
    return GERIUM_VERSION_STRING;
}

gerium_utf8_t gerium_state_to_string(gerium_state_t state) {
    switch (state) {
        case GERIUM_STATE_SUCCESS:
            return "no error has occurred";

        case GERIUM_STATE_UNKNOWN_ERROR:
            return "out of memory";

        case GERIUM_STATE_OUT_OF_MEMORY:
            return "unknown error";

        case GERIUM_STATE_NOT_IMPLEMENTED:
            return "not implemented";

        case GERIUM_STATE_FEATURE_NOT_SUPPORTED:
            return "feature not supported";

        case GERIUM_STATE_INVALID_ARGUMENT:
            return "invalid argument passed";

        default:
            return "<unknown state>";
    }
}
