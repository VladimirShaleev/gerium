#ifndef GERIUM_MAC_OS_MAC_OS_FILE_HPP
#define GERIUM_MAC_OS_MAC_OS_FILE_HPP

#include "../Unix/UnixFile.hpp"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

namespace gerium::macos {

class MacOSFile final : unix::UnixFile {
public:
    explicit MacOSFile(gerium_uint64_t size);
    MacOSFile(gerium_utf8_t path, gerium_uint64_t size);
    MacOSFile(gerium_utf8_t path, bool readOnly);

    static std::string getPath(NSSearchPathDirectory directory) noexcept;
    static bool exists(gerium_utf8_t path, bool isDir) noexcept;

private:
    static std::string getTempFile();
    static NSString* appendBundleId(NSString* path);
};

} // namespace gerium::macos

#endif
