#include "iOSLogger.hpp"

namespace gerium::ios {

iOSLogger::iOSLogger(gerium_utf8_t tag) : Logger(tag) {
}

void iOSLogger::onPrint(const std::string& tag, gerium_logger_level_t level, gerium_utf8_t message) {
    NSLog(@"[%s] %@: %s", tag.c_str(), getLogLevel(level), message);
}

NSString* iOSLogger::getLogLevel(gerium_logger_level_t level) noexcept {
    switch (level) {
        case GERIUM_LOGGER_LEVEL_VERBOSE:
            return @"VERBOSE";
        case GERIUM_LOGGER_LEVEL_DEBUG:
            return @"DEBUG";
        case GERIUM_LOGGER_LEVEL_INFO:
            return @"INFO";
        case GERIUM_LOGGER_LEVEL_WARNING:
            return @"WARNING";
        case GERIUM_LOGGER_LEVEL_ERROR:
            return @"ERROR";
        case GERIUM_LOGGER_LEVEL_FATAL:
            return @"FATAL";
        case GERIUM_LOGGER_LEVEL_OFF:
            return @"OFF";
        default:
            return @"VERBOSE";
    }
}

} // namespace gerium::ios

gerium_result_t gerium_logger_create(gerium_utf8_t tag, gerium_logger_t* logger) {
    using namespace gerium;
    using namespace gerium::ios;
    return Object::create<iOSLogger>(*logger, tag);
}
