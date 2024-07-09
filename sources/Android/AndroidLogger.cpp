#include "AndroidLogger.hpp"
#include <android/log.h>

namespace gerium::android {

AndroidLogger::AndroidLogger(gerium_utf8_t tag) : Logger(tag) {
}

void AndroidLogger::onPrint(const std::string& tag, gerium_logger_level_t level, gerium_utf8_t message) {
    const auto priority = ANDROID_LOG_VERBOSE + (int) level;
    __android_log_print(priority, tag.c_str(), "%s", message);
}

} // namespace gerium::android

gerium_result_t gerium_logger_create(gerium_utf8_t tag, gerium_logger_t* logger) {
    using namespace gerium;
    using namespace gerium::android;
    return Object::create<AndroidLogger>(*logger, tag);
}
