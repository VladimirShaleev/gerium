#ifndef GERIUM_FILE_HPP
#define GERIUM_FILE_HPP

#include "ObjectPtr.hpp"

struct _gerium_file : public gerium::Object {};

namespace gerium {

class File : public _gerium_file {
public:
    gerium_uint32_t getSize() noexcept;
    void write(gerium_cdata_t data, gerium_uint32_t size);
    gerium_uint32_t read(gerium_data_t data, gerium_uint32_t size) noexcept;
    gerium_data_t map() noexcept;

    static ObjectPtr<File> create(gerium_utf8_t path, gerium_uint32_t size);

private:
    virtual gerium_uint32_t onGetSize() noexcept                                      = 0;
    virtual void onWrite(gerium_cdata_t data, gerium_uint32_t size)                   = 0;
    virtual gerium_uint32_t onRead(gerium_data_t data, gerium_uint32_t size) noexcept = 0;
    virtual gerium_data_t onMap() noexcept                                            = 0;
};

} // namespace gerium

#endif
