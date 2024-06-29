#include "MacOSLogger.hpp"

namespace gerium::macos {

MacOSLogger::MacOSLogger(gerium_utf8_t tag) : Logger(tag) {
}

void MacOSLogger::onPrint(const std::string& tag, gerium_logger_level_t level, gerium_utf8_t message) {
    NSLog(@"[%s] %@: %s", tag.c_str(), getLogLevel(level), message);
}

NSString* MacOSLogger::getLogLevel(gerium_logger_level_t level) noexcept {
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
        default:
            return @"VERBOSE";
    }
}

} // namespace gerium::macos

gerium_result_t gerium_logger_create(gerium_utf8_t tag, gerium_logger_t* logger) {
    using namespace gerium;
    using namespace gerium::macos;
    return Object::create<MacOSLogger>(*logger, tag);
}
