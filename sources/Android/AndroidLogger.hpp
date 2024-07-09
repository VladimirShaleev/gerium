#ifndef GERIUM_ANDROID_ANDROID_LOGGER_HPP
#define GERIUM_ANDROID_ANDROID_LOGGER_HPP

#include "../Logger.hpp"

namespace gerium::android {

class AndroidLogger final : public Logger {
public:
    explicit AndroidLogger(gerium_utf8_t tag);

protected:
    void onPrint(const std::string& tag, gerium_logger_level_t level, gerium_utf8_t message) override;
};

} // namespace gerium::android

#endif
