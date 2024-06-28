#ifndef GERIUM_WINDOWS_UNICODE_HPP
#define GERIUM_WINDOWS_UNICODE_HPP

#include "../Gerium.hpp"

namespace gerium::windows {

gerium_inline std::wstring wideString(gerium_utf8_t utf8) {
    auto byteCount = (int) std::strlen(utf8);
    std::wstring result;
    result.resize(byteCount * 4 + 1);
    MultiByteToWideChar(CP_UTF8, 0, (LPCCH) utf8, byteCount, result.data(), (int) result.size());
    return result;
}

gerium_inline std::wstring wideString(const std::string& utf8) {
    auto byteCount = (int) utf8.length();
    std::wstring result;
    result.resize(byteCount * 4 + 1);
    MultiByteToWideChar(CP_UTF8, 0, (LPCCH) utf8.data(), byteCount, result.data(), (int) result.size());
    return result;
}

gerium_inline std::string utf8String(const std::wstring& wstr) {
    std::string result;
    result.resize(wstr.length() * 2);
    WideCharToMultiByte(
        CP_UTF8, 0, (LPCWCH) wstr.data(), wstr.length(), result.data(), (int) result.size(), nullptr, nullptr);
    return result;
}

} // namespace gerium::windows

#endif
