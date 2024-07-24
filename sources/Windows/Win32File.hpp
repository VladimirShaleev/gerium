#ifndef GERIUM_WINDOWS_WIN32_FILE_HPP
#define GERIUM_WINDOWS_WIN32_FILE_HPP

#include "../File.hpp"

namespace gerium::windows {

class Win32File final : public File {
public:
    Win32File(gerium_uint32_t size);
    Win32File(gerium_utf8_t path, gerium_uint32_t size);
    Win32File(gerium_utf8_t path, bool readOnly);

    ~Win32File();

private:
    gerium_uint32_t onGetSize() noexcept override;
    void onWrite(gerium_cdata_t data, gerium_uint32_t size) override;
    gerium_uint32_t onRead(gerium_data_t data, gerium_uint32_t size) noexcept override;
    gerium_data_t onMap() noexcept override;

    static std::string getTempFileName();

    HANDLE _file;
    HANDLE _map;
    gerium_data_t _data;
    bool _readOnly;
};

} // namespace gerium::windows

#endif
