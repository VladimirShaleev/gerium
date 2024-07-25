#include "MacOSFile.hpp"

namespace gerium {

namespace macos {

MacOSFile::MacOSFile(gerium_uint64_t size) : MacOSFile("<todo>", size) {
}

MacOSFile::MacOSFile(gerium_utf8_t path, gerium_uint64_t size) : File(false) {
}

MacOSFile::MacOSFile(gerium_utf8_t path, bool readOnly) : File(readOnly) {
}

MacOSFile::~MacOSFile() {
}

gerium_uint64_t MacOSFile::onGetSize() noexcept {
}

void MacOSFile::onSeek(gerium_uint64_t offset, gerium_file_seek_t seek) noexcept {
}

void MacOSFile::onWrite(gerium_cdata_t data, gerium_uint32_t size) {
}

gerium_uint32_t MacOSFile::onRead(gerium_data_t data, gerium_uint32_t size) noexcept {
}

gerium_data_t MacOSFile::onMap() noexcept {
}

} // namespace macos

gerium_utf8_t File::getCacheDir() noexcept {
}

gerium_utf8_t File::getAppDir() noexcept {
}

bool File::existsFile(gerium_utf8_t path) noexcept {
}

bool File::existsDir(gerium_utf8_t path) noexcept {
}

void File::deleteFile(gerium_utf8_t path) noexcept {
}

} // namespace gerium

using namespace gerium;
using namespace gerium::macos;

gerium_result_t gerium_file_open(gerium_utf8_t path, gerium_bool_t read_only, gerium_file_t* file) {
    return Object::create<MacOSFile>(*file, path, read_only != 0);
}

gerium_result_t gerium_file_create(gerium_utf8_t path, gerium_uint64_t size, gerium_file_t* file) {
    return Object::create<MacOSFile>(*file, path, size);
}

gerium_result_t gerium_file_create_temp(gerium_uint64_t size, gerium_file_t* file) {
    return Object::create<MacOSFile>(*file, size);
}
