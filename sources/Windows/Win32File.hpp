#ifndef GERIUM_WINDOWS_WIN32_FILE_HPP
#define GERIUM_WINDOWS_WIN32_FILE_HPP

#include "../File.hpp"

namespace gerium::windows {

class Win32File final : public File {
public:
    Win32File(gerium_uint32_t size);
    explicit Win32File(gerium_utf8_t path, gerium_uint32_t size);

    ~Win32File();

private:
    static std::string getTempFileName();

    HANDLE _file;
};

} // namespace gerium::windows

#endif
