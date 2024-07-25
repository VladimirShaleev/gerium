#include "MacOSFile.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace gerium {

namespace macos {

MacOSFile::MacOSFile(gerium_uint64_t size) : MacOSFile(getTempFile().c_str(), size) {
}

MacOSFile::MacOSFile(gerium_utf8_t path, gerium_uint64_t size) : File(false), _file(-1), _data(nullptr) {
    _file = ::open(path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (_file < 0) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }

    if (size) {
        fstore_t store = { F_ALLOCATECONTIG, F_PEOFPOSMODE, 0, (off_t) size };
        if (fcntl(_file, F_PREALLOCATE, &store) == -1) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }
        if (ftruncate(_file, (off_t) size) != 0) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }
    }
}

MacOSFile::MacOSFile(gerium_utf8_t path, bool readOnly) : File(readOnly), _file(-1), _data(nullptr) {
    _file = ::open(path, (readOnly ? O_RDONLY : O_RDWR) | O_CREAT, S_IRUSR | (readOnly ? 0 : S_IWUSR));

    if (_file < 0) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }
}

MacOSFile::~MacOSFile() {
    if (_data) {
        ::munmap(_data, onGetSize());
    }

    if (_file >= 0) {
        ::close(_file);
    }
}

std::string MacOSFile::getPath(NSSearchPathDirectory directory) noexcept {
    std::string dir;
    NSArray* paths = NSSearchPathForDirectoriesInDomains(directory, NSUserDomainMask, YES);
    if ([paths count] > 0) {
        NSString* firstPath = [paths objectAtIndex:0];
        NSString* path = appendBundleId(firstPath);
        dir = [path UTF8String];
    }
    return dir;
}

bool MacOSFile::exists(gerium_utf8_t path, bool isDir) noexcept {
    NSString* file = [NSString stringWithUTF8String:path];
    BOOL isDirResult = NO;
    BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:file isDirectory:&isDirResult];
    return exists && (isDirResult != NO) == isDir;
}

std::string MacOSFile::getTempFile() {
    NSString* dir = appendBundleId(NSTemporaryDirectory());
    NSString* file = [dir stringByAppendingPathComponent:[[NSUUID UUID] UUIDString].lowercaseString];
    NSString* fileExt = [file stringByAppendingString:@".tmp"];
    return [fileExt UTF8String];
}

NSString* MacOSFile::appendBundleId(NSString* path) {
    NSString* dir = [path stringByAppendingPathComponent:[[NSBundle mainBundle] bundleIdentifier]];
    if ([dir compare:path] == NSOrderedSame) {
        dir = [path stringByAppendingPathComponent:[[NSProcessInfo processInfo] processName]];
    }
    return dir;
}

gerium_uint64_t MacOSFile::onGetSize() noexcept {
    struct stat stat {};

    ::fstat(_file, &stat);
    return (gerium_uint64_t) stat.st_size;
}

void MacOSFile::onSeek(gerium_uint64_t offset, gerium_file_seek_t seek) noexcept {
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
    ::lseek(_file, off_t(offset), seek);
}

void MacOSFile::onWrite(gerium_cdata_t data, gerium_uint32_t size) {
    if (::write(_file, (const void*) data, (size_t) size) < 0) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }
}

gerium_uint32_t MacOSFile::onRead(gerium_data_t data, gerium_uint32_t size) noexcept {
    return (gerium_uint32_t)::read(_file, (void*) data, (size_t) size);
}

gerium_data_t MacOSFile::onMap() noexcept {
    if (!_data) {
        _data = ::mmap(nullptr,
                       onGetSize(),
                       PROT_READ | (isReadOnly() ? 0 : PROT_WRITE),
                       isReadOnly() ? MAP_PRIVATE : MAP_SHARED,
                       _file,
                       0);
    }
    return _data;
}

} // namespace macos

gerium_utf8_t File::getCacheDir() noexcept {
    static auto dir = macos::MacOSFile::getPath(NSCachesDirectory);
    return dir.c_str();
}

gerium_utf8_t File::getAppDir() noexcept {
    static auto dir = macos::MacOSFile::getPath(NSApplicationSupportDirectory);
    return dir.c_str();
}

bool File::existsFile(gerium_utf8_t path) noexcept {
    return macos::MacOSFile::exists(path, false);
}

bool File::existsDir(gerium_utf8_t path) noexcept {
    return macos::MacOSFile::exists(path, true);
}

void File::deleteFile(gerium_utf8_t path) noexcept {
    if (macos::MacOSFile::exists(path, false)) {
        NSString* file = [NSString stringWithUTF8String:path];
        [[NSFileManager defaultManager] removeItemAtPath:file error:nil];
    }
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
