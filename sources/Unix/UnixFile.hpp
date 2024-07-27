#ifndef GERIUM_UNIX_UNIX_FILE_HPP
#define GERIUM_UNIX_UNIX_FILE_HPP

#include "../File.hpp"

namespace gerium::unix {

class UnixFile : public File {
public:
    UnixFile(gerium_utf8_t path, gerium_uint64_t size);
    UnixFile(gerium_utf8_t path, bool readOnly);

    ~UnixFile() override;

private:
    void reserveSpace(gerium_uint64_t size) const;
    static void createDirs(gerium_utf8_t path);

    gerium_uint64_t onGetSize() noexcept override;
    void onSeek(gerium_uint64_t offset, gerium_file_seek_t seek) noexcept override;
    void onWrite(gerium_cdata_t data, gerium_uint32_t size) override;
    gerium_uint32_t onRead(gerium_data_t data, gerium_uint32_t size) noexcept override;
    gerium_data_t onMap() noexcept override;

    int _file;
    gerium_data_t _data;
};

} // namespace gerium::unix

#endif
