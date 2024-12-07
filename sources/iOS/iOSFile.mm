#include "iOSFile.hpp"

#include <mach-o/dyld.h>

namespace gerium {

namespace ios {

iOSFile::iOSFile(gerium_uint64_t size) : iOSFile(getTempFile().c_str(), size) {
}

iOSFile::iOSFile(gerium_utf8_t path, gerium_uint64_t size) : nix::UnixFile(path, size) {
}

iOSFile::iOSFile(gerium_utf8_t path, bool readOnly) :
    nix::UnixFile(pathFromResources(path, readOnly), readOnly) {
}

std::string iOSFile::getPath(NSSearchPathDirectory directory) noexcept {
    std::string dir;
    NSArray* paths = NSSearchPathForDirectoriesInDomains(directory, NSUserDomainMask, YES);
    if ([paths count] > 0) {
        NSString* firstPath = [paths objectAtIndex:0];
        NSString* path      = appendBundleId(firstPath);
        dir                 = [path UTF8String];
    }
    return dir;
}

bool iOSFile::exists(gerium_utf8_t path, bool isDir) noexcept {
    NSString* file   = [NSString stringWithUTF8String:path];
    BOOL isDirResult = NO;
    BOOL exists      = [[NSFileManager defaultManager] fileExistsAtPath:file isDirectory:&isDirResult];
    return exists && (isDirResult != NO) == isDir;
}

bool iOSFile::resourceExists(gerium_utf8_t path) noexcept {
    auto appDir   = getAppDir();
    auto relative = std::filesystem::relative(path, appDir);
    auto ext      = relative.extension().string().substr(1);
    auto name     = relative.replace_extension().string();

    NSString* extension = [NSString stringWithUTF8String:ext.c_str()];
    NSString* fileName  = [NSString stringWithUTF8String:name.c_str()];
    NSString* filePath  = [[NSBundle mainBundle] pathForResource:fileName ofType:extension];

    return filePath != nil;
}

std::string iOSFile::getTempFile() {
    NSString* dir     = appendBundleId(NSTemporaryDirectory());
    NSString* file    = [dir stringByAppendingPathComponent:[[NSUUID UUID] UUIDString].lowercaseString];
    NSString* fileExt = [file stringByAppendingString:@".tmp"];
    return [fileExt UTF8String];
}

NSString* iOSFile::appendBundleId(NSString* path) {
    NSString* dir = [path stringByAppendingPathComponent:[[NSBundle mainBundle] bundleIdentifier]];
    if ([dir compare:path] == NSOrderedSame) {
        dir = [path stringByAppendingPathComponent:[[NSProcessInfo processInfo] processName]];
    }
    return dir;
}

gerium_utf8_t iOSFile::pathFromResources(gerium_utf8_t path, bool readOnly) {
    if (readOnly && !exists(path, false)) {
        auto appDir   = getAppDir();
        auto relative = std::filesystem::relative(path, appDir);
        auto ext      = relative.extension().string().substr(1);
        auto name     = relative.replace_extension().string();

        NSString* extension = [NSString stringWithUTF8String:ext.c_str()];
        NSString* fileName  = [NSString stringWithUTF8String:name.c_str()];
        NSString* filePath  = [[NSBundle mainBundle] pathForResource:fileName ofType:extension];

        if (filePath == nil) {
            return path;
        }

        static std::string resourcePath;
        resourcePath = [filePath UTF8String];
        return resourcePath.c_str();
    }
    return path;
}

} // namespace ios

gerium_utf8_t File::getCacheDir() noexcept {
    static auto dir = ios::iOSFile::getPath(NSCachesDirectory);
    return dir.c_str();
}

gerium_utf8_t File::getAppDir() noexcept {
    static auto dir = ios::iOSFile::getPath(NSApplicationSupportDirectory);
    return dir.c_str();
}

bool File::existsFile(gerium_utf8_t path) noexcept {
    return ios::iOSFile::exists(path, false) || ios::iOSFile::resourceExists(path);
}

bool File::existsDir(gerium_utf8_t path) noexcept {
    return ios::iOSFile::exists(path, true);
}

void File::deleteFile(gerium_utf8_t path) noexcept {
    if (ios::iOSFile::exists(path, false)) {
        NSString* file = [NSString stringWithUTF8String:path];
        [[NSFileManager defaultManager] removeItemAtPath:file error:nil];
    }
}

} // namespace gerium

using namespace gerium;
using namespace gerium::ios;

gerium_result_t gerium_file_open(gerium_utf8_t path, gerium_bool_t read_only, gerium_file_t* file) {
    return Object::create<iOSFile>(*file, path, read_only != 0);
}

gerium_result_t gerium_file_create(gerium_utf8_t path, gerium_uint64_t size, gerium_file_t* file) {
    return Object::create<iOSFile>(*file, path, size);
}

gerium_result_t gerium_file_create_temp(gerium_uint64_t size, gerium_file_t* file) {
    return Object::create<iOSFile>(*file, size);
}
