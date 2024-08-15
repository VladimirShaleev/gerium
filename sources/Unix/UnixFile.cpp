#include "UnixFile.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace gerium::unix {

UnixFile::UnixFile(gerium_utf8_t path, gerium_uint64_t size) : File(false), _file(-1), _data(nullptr), _dataSize(0) {
    createDirs(path);

    _file = ::open(path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (_file < 0) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }

    reserveSpace(size);
}

UnixFile::UnixFile(gerium_utf8_t path, bool readOnly) : File(readOnly), _file(-1), _data(nullptr), _dataSize(0) {
    createDirs(path);

    _file = ::open(path, (readOnly ? O_RDONLY : O_RDWR) | O_CREAT, S_IRUSR | (readOnly ? 0 : S_IWUSR));

    if (_file < 0) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }
}

UnixFile::~UnixFile() {
    if (_data) {
        ::munmap(_data, _dataSize);
    }

    if (_file >= 0) {
        ::close(_file);
    }
}

void UnixFile::createDirs(gerium_utf8_t path) {
    if (!std::filesystem::exists(path)) {
        const auto dir = std::filesystem::path(path).parent_path();
        auto currentPath = *dir.begin();
        for (auto it = ++dir.begin(); it != dir.end(); ++it) {
            currentPath /= *it;
            ::mkdir(currentPath.string().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
    }
}

void UnixFile::reserveSpace(gerium_uint64_t size) const {
    if (size) {
#ifdef __APPLE__
        fstore_t store = { F_ALLOCATECONTIG, F_PEOFPOSMODE, 0, (off_t) size };
        if (fcntl(_file, F_PREALLOCATE, &store) == -1) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }
        if (ftruncate(_file, (off_t) size) != 0) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }
#else
        if (::fallocate64(_file, 0, 0, (off64_t) size) != 0) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }
#endif
    }
}

gerium_uint64_t UnixFile::onGetSize() noexcept {
#ifdef __APPLE__
    struct stat stat {};

    ::fstat(_file, &stat);
#else
    struct stat64 stat {};

    ::fstat64(_file, &stat);
#endif
    return (gerium_uint64_t) stat.st_size;
}

void UnixFile::onSeek(gerium_uint64_t offset, gerium_file_seek_t seek) noexcept {
    int whence;
    switch (seek) {
        case GERIUM_FILE_SEEK_BEGIN:
            whence = SEEK_SET;
            break;
        case GERIUM_FILE_SEEK_CURRENT:
            whence = SEEK_CUR;
            break;
        case GERIUM_FILE_SEEK_END:
            whence = SEEK_END;
            break;
        default:
            assert(!"unreachable code");
            whence = SEEK_SET;
            break;
    }
#ifdef __APPLE__
    ::lseek(_file, off_t(offset), seek);
#else
    ::lseek64(_file, off64_t(offset), seek);
#endif
}

void UnixFile::onWrite(gerium_cdata_t data, gerium_uint32_t size) {
    if (::write(_file, (const void*) data, (size_t) size) < 0) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }
}

gerium_uint32_t UnixFile::onRead(gerium_data_t data, gerium_uint32_t size) noexcept {
    return (gerium_uint32_t)::read(_file, (void*) data, (size_t) size);
}

gerium_data_t UnixFile::onMap() noexcept {
    if (!_data) {
        _dataSize = onGetSize();
#ifdef __APPLE__
# define os_mmap ::mmap
#else
# define os_mmap ::mmap64
#endif
        _data = os_mmap(nullptr,
                        _dataSize,
                        PROT_READ | (isReadOnly() ? 0 : PROT_WRITE),
                        isReadOnly() ? MAP_PRIVATE : MAP_SHARED,
                        _file,
                        0);
#undef os_mmap
    }
    return _data;
}

} // namespace gerium::unix
