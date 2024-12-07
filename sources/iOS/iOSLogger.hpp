#ifndef GERIUM_IOS_IOS_LOGGER_HPP
#define GERIUM_IOS_IOS_LOGGER_HPP

#include "../Logger.hpp"

#import <Foundation/Foundation.h>

namespace gerium::ios {

class iOSLogger final : public Logger {
public:
    explicit iOSLogger(gerium_utf8_t tag);

protected:
    void onPrint(const std::string& tag, gerium_logger_level_t level, gerium_utf8_t message) override;

private:
    static NSString* getLogLevel(gerium_logger_level_t level) noexcept;
};

} // namespace gerium::ios

#endif
