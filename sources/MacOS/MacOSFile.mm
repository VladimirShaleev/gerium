#include "MacOSFile.hpp"

#include <mach-o/dyld.h>

namespace gerium {

namespace macos {

MacOSFile::MacOSFile(gerium_uint64_t size) : MacOSFile(getTempFile().c_str(), size) {
}

MacOSFile::MacOSFile(gerium_utf8_t path, gerium_uint64_t size) : unix::UnixFile(path, size) {
}

MacOSFile::MacOSFile(gerium_utf8_t path, bool readOnly) : unix::UnixFile(path, readOnly) {
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

} // namespace macos

gerium_utf8_t File::getCacheDir() noexcept {
    static auto dir = macos::MacOSFile::getPath(NSCachesDirectory);
    return dir.c_str();
}

gerium_utf8_t File::getAppDir() noexcept {
    //static auto dir = macos::MacOSFile::getPath(NSApplicationSupportDirectory);
    //return dir.c_str();
    static std::string dir;
    char path[1025]{};
    uint32_t size = 1024;
    if (_NSGetExecutablePath(path, &size) == 0) {
        dir = std::filesystem::path(path).parent_path().parent_path().parent_path().parent_path().string();
    }
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
