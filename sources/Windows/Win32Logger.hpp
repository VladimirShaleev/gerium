#ifndef GERIUM_WINDOWS_WIN32_LOGGER_HPP
#define GERIUM_WINDOWS_WIN32_LOGGER_HPP

#include "../Logger.hpp"

namespace gerium::windows {

class Win32Logger final : public Logger {
public:
    explicit Win32Logger(gerium_utf8_t tag);

protected:
    void onPrint(const std::string& tag, gerium_logger_level_t level, gerium_utf8_t message) override;

private:
    bool _isDebugger;
};

} // namespace gerium::windows

#endif
