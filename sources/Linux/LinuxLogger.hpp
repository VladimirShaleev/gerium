#ifndef GERIUM_LINUX_LINUX_LOGGER_HPP
#define GERIUM_LINUX_LINUX_LOGGER_HPP

#include "../Logger.hpp"

namespace gerium::linux {

class LinuxLogger final : public Logger {
public:
    explicit LinuxLogger(gerium_utf8_t tag);

protected:
    void onPrint(const std::string& tag, gerium_logger_level_t level, gerium_utf8_t message) override;

private:
    static int getLogLevel(gerium_logger_level_t level) noexcept;
};

} // namespace gerium::linux

#endif
