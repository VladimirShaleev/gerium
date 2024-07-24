#include "File.hpp"

namespace gerium {

ObjectPtr<File> File::create(gerium_utf8_t path, gerium_uint32_t size) {
    gerium_file_t file = nullptr;
    if (auto result = gerium_file_create(path, size, &file); result != GERIUM_RESULT_SUCCESS) {
        error(result);
    }
    return ObjectPtr(alias_cast<File*>(file), false);
}

} // namespace gerium

gerium_file_t gerium_file_reference(gerium_file_t file) {
    assert(file);
    file->reference();
    return file;
}

void gerium_file_destroy(gerium_file_t file) {
    if (file) {
        file->destroy();
    }
}

gerium_uint32_t gerium_file_get_size(gerium_file_t file) {
    return 0; // TODO: add impl
}
