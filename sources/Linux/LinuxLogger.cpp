#include "LinuxLogger.hpp"

#include <syslog.h>

namespace gerium::linux {

LinuxLogger::LinuxLogger(gerium_utf8_t tag) : Logger(tag) {
    openlog(tag, LOG_PID, LOG_LOCAL0);
}

void LinuxLogger::onPrint(const std::string& tag, gerium_logger_level_t level, gerium_utf8_t message) {
#ifdef NDEBUG
    int priority = getLogLevel(level);
    syslog(getLogLevel(level), "%s", message);
#else
    using namespace std::string_view_literals;
    std::cout << '[' << tag << "] "sv << levelToString(level) << ": "sv << message << std::endl;
#endif
}

int LinuxLogger::getLogLevel(gerium_logger_level_t level) noexcept {
    switch (level) {
        case GERIUM_LOGGER_LEVEL_VERBOSE:
            return LOG_DEBUG;
        case GERIUM_LOGGER_LEVEL_DEBUG:
            return LOG_DEBUG;
        case GERIUM_LOGGER_LEVEL_INFO:
            return LOG_INFO;
        case GERIUM_LOGGER_LEVEL_WARNING:
            return LOG_WARNING;
        case GERIUM_LOGGER_LEVEL_ERROR:
            return LOG_ERR;
        case GERIUM_LOGGER_LEVEL_FATAL:
            return LOG_CRIT;
        default:
            return LOG_NOTICE;
    }
}

} // namespace gerium::linux

gerium_result_t gerium_logger_create(gerium_utf8_t tag, gerium_logger_t* logger) {
    using namespace gerium;
    using namespace gerium::linux;
    return Object::create<LinuxLogger>(*logger, tag);
}
