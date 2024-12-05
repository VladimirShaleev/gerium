#include "LinuxFile.hpp"

#include <fstream>

namespace gerium {

namespace linux {

LinuxFile::LinuxFile(gerium_uint64_t size) : LinuxFile(createTempFile().c_str(), size) {
}

LinuxFile::LinuxFile(gerium_utf8_t path, gerium_uint64_t size) : nix::UnixFile(path, size) {
}

LinuxFile::LinuxFile(gerium_utf8_t path, bool readOnly) : nix::UnixFile(path, readOnly) {
}

LinuxFile::~LinuxFile() {
    if (_file) {
        fclose(_file);
        _file = nullptr;
    }
}

std::string LinuxFile::createTempFile() {
    _file = tmpfile();

    auto fd = fileno(_file);
    char buf[128];
    snprintf(buf, 128, "/proc/self/fd/%d", fd);

    char path[PATH_MAX];
    readlink(buf, path, PATH_MAX);

    return path;
}

} // namespace linux

gerium_utf8_t File::getCacheDir() noexcept {
    static std::string dir;
    if (dir.empty()) {
        if (auto xdgCache = std::getenv("XDG_CACHE_HOME")) {
            dir = std::string(xdgCache) + "/";
        } else if (auto home = std::getenv("HOME")) {
            dir = std::string(home) + "/.cache/";
        }
        std::string sp;
        std::ifstream("/proc/self/comm") >> sp;
        dir += sp + "/";
    }
    return dir.c_str();
}

gerium_utf8_t File::getAppDir() noexcept {
    static std::string dir;
    if (dir.empty()) {
        char buf[PATH_MAX];
        readlink("/proc/self/exe", buf, PATH_MAX);
        dir = std::filesystem::path(buf).parent_path();
    }
    return dir.c_str();
}

bool File::existsFile(gerium_utf8_t path) noexcept {
    std::filesystem::path file = path;
    return std::filesystem::exists(file) && !std::filesystem::is_directory(file);
}

bool File::existsDir(gerium_utf8_t path) noexcept {
    std::filesystem::path dir = path;
    return std::filesystem::exists(dir) && std::filesystem::is_directory(dir);
}

void File::deleteFile(gerium_utf8_t path) noexcept {
    if (existsFile(path)) {
        std::filesystem::remove(path);
    }
}

} // namespace gerium

using namespace gerium;
using namespace gerium::linux;

gerium_result_t gerium_file_open(gerium_utf8_t path, gerium_bool_t read_only, gerium_file_t* file) {
    return Object::create<LinuxFile>(*file, path, read_only != 0);
}

gerium_result_t gerium_file_create(gerium_utf8_t path, gerium_uint64_t size, gerium_file_t* file) {
    return Object::create<LinuxFile>(*file, path, size);
}

gerium_result_t gerium_file_create_temp(gerium_uint64_t size, gerium_file_t* file) {
    return Object::create<LinuxFile>(*file, size);
}
