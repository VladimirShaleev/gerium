#include "Win32Logger.hpp"
#include "Unicode.hpp"

namespace gerium::windows {

Win32Logger::Win32Logger(gerium_utf8_t tag) : Logger(tag), _isDebugger(IsDebuggerPresent()) {
}

void Win32Logger::onPrint(const std::string& tag, gerium_logger_level_t level, gerium_utf8_t message) {
    using namespace std::string_view_literals;

    std::ostringstream ss;
    ss << '[' << tag << "] "sv << levelToString(level) << ": "sv << message << std::endl;

    const auto str = ss.str();

    if (_isDebugger) {
        const auto wstr = wideString(str);
        OutputDebugStringW(wstr.data());
    } else {
        printf("%s", str.data());
    }
}

} // namespace gerium::windows

gerium_result_t gerium_logger_create(gerium_utf8_t tag, gerium_logger_t* logger) {
    using namespace gerium;
    using namespace gerium::windows;
    return Object::create<Win32Logger>(*logger, tag);
}
