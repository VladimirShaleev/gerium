#ifndef GERIUM_MAC_OS_MAC_OS_LOGGER_HPP
#define GERIUM_MAC_OS_MAC_OS_LOGGER_HPP

#include "../Logger.hpp"

#import <Foundation/Foundation.h>

namespace gerium::macos {

class MacOSLogger final : public Logger {
public:
    explicit MacOSLogger(gerium_utf8_t tag);

protected:
    void onPrint(const std::string& tag, gerium_logger_level_t level, gerium_utf8_t message) override;

private:
    static NSString* getLogLevel(gerium_logger_level_t level) noexcept;
};

} // namespace gerium::macos

#endif
