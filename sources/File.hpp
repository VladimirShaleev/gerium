#ifndef GERIUM_FILE_HPP
#define GERIUM_FILE_HPP

#include "ObjectPtr.hpp"

struct _gerium_file : public gerium::Object {};

namespace gerium {

class File : public _gerium_file {
public:
    static ObjectPtr<File> create(gerium_utf8_t path, gerium_uint32_t size);
};

} // namespace gerium

#endif
