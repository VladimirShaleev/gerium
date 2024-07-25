#ifndef GERIUM_FILE_HPP
#define GERIUM_FILE_HPP

#include "ObjectPtr.hpp"

struct _gerium_file : public gerium::Object {};

namespace gerium {

class File : public _gerium_file {
public:
    explicit File(bool readOnly) noexcept;

    [[nodiscard]] bool isReadOnly() const noexcept;
    [[nodiscard]] gerium_uint64_t getSize() noexcept;
    void seek(gerium_uint64_t offset, gerium_file_seek_t seek) noexcept;
    void write(gerium_cdata_t data, gerium_uint32_t size);
    gerium_uint32_t read(gerium_data_t data, gerium_uint32_t size) noexcept;
    [[nodiscard]] gerium_data_t map() noexcept;

    [[nodiscard]] static ObjectPtr<File> open(gerium_utf8_t path, bool readOnly);
    [[nodiscard]] static ObjectPtr<File> create(gerium_utf8_t path, gerium_uint32_t size);

    [[nodiscard]] static gerium_utf8_t getCacheDir() noexcept;
    [[nodiscard]] static gerium_utf8_t getAppDir() noexcept;

    [[nodiscard]] static bool existsFile(gerium_utf8_t path) noexcept;
    [[nodiscard]] static bool existsDir(gerium_utf8_t path) noexcept;
    static void deleteFile(gerium_utf8_t path) noexcept;

private:
    virtual gerium_uint64_t onGetSize() noexcept                                      = 0;
    virtual void onSeek(gerium_uint64_t offset, gerium_file_seek_t seek) noexcept     = 0;
    virtual void onWrite(gerium_cdata_t data, gerium_uint32_t size)                   = 0;
    virtual gerium_uint32_t onRead(gerium_data_t data, gerium_uint32_t size) noexcept = 0;
    virtual gerium_data_t onMap() noexcept                                            = 0;

    bool _readOnly;
};

} // namespace gerium

#endif
