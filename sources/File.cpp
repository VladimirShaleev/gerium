#include "File.hpp"

namespace gerium {

gerium_uint64_t File::getSize() noexcept {
    return onGetSize();
}

void File::seek(gerium_uint64_t offset, gerium_file_seek_t seek) noexcept {
    onSeek(offset, seek);
}

void File::write(gerium_cdata_t data, gerium_uint32_t size) {
    onWrite(data, size);
}

gerium_uint32_t File::read(gerium_data_t data, gerium_uint32_t size) noexcept {
    return onRead(data, size);
}

gerium_data_t File::map() noexcept {
    return onMap();
}

ObjectPtr<File> File::open(gerium_utf8_t path, bool readOnly) {
    gerium_file_t file = nullptr;
    if (auto result = gerium_file_open(path, readOnly, &file); result != GERIUM_RESULT_SUCCESS) {
        error(result);
    }
    return ObjectPtr(alias_cast<File*>(file), false);
}

ObjectPtr<File> File::create(gerium_utf8_t path, gerium_uint32_t size) {
    gerium_file_t file = nullptr;
    if (auto result = gerium_file_create(path, size, &file); result != GERIUM_RESULT_SUCCESS) {
        error(result);
    }
    return ObjectPtr(alias_cast<File*>(file), false);
}

bool File::existsFile(gerium_utf8_t path) noexcept {
    return gerium_file_exists_file(path);
}

bool File::existsDir(gerium_utf8_t path) noexcept {
    return gerium_file_exists_dir(path);
}

void File::deleteFile(gerium_utf8_t path) noexcept {
    return gerium_file_delete_file(path);
}

} // namespace gerium

using namespace gerium;

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

gerium_uint64_t gerium_file_get_size(gerium_file_t file) {
    assert(file);
    return alias_cast<File*>(file)->getSize();
}

void gerium_file_seek(gerium_file_t file, gerium_uint64_t offset, gerium_file_seek_t seek) {
    assert(file);
    assert(seek >= GERIUM_FILE_SEEK_BEGIN && seek <= GERIUM_FILE_SEEK_END);
    alias_cast<File*>(file)->seek(offset, seek);
}

gerium_result_t gerium_file_write(gerium_file_t file, gerium_cdata_t data, gerium_uint32_t size) {
    assert(file);
    assert(data);
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<File*>(file)->write(data, size);
    GERIUM_END_SAFE_BLOCK
}

gerium_uint32_t gerium_file_read(gerium_file_t file, gerium_data_t data, gerium_uint32_t size) {
    assert(file);
    assert(data);
    return alias_cast<File*>(file)->read(data, size);
}

gerium_data_t gerium_file_map(gerium_file_t file) {
    assert(file);
    return alias_cast<File*>(file)->map();
}
