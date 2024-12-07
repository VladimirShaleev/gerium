#ifndef GERIUM_IOS_IOS_FILE_HPP
#define GERIUM_IOS_IOS_FILE_HPP

#include "../Unix/UnixFile.hpp"

#import <Foundation/Foundation.h>

namespace gerium::ios {

class iOSFile final : public nix::UnixFile {
public:
    explicit iOSFile(gerium_uint64_t size);
    iOSFile(gerium_utf8_t path, gerium_uint64_t size);
    iOSFile(gerium_utf8_t path, bool readOnly);

    static std::string getPath(NSSearchPathDirectory directory) noexcept;
    static bool exists(gerium_utf8_t path, bool isDir) noexcept;
    static bool resourceExists(gerium_utf8_t path) noexcept;

private:
    static std::string getTempFile();
    static NSString* appendBundleId(NSString* path);
    static gerium_utf8_t pathFromResources(gerium_utf8_t path, bool readOnly);
};

} // namespace gerium::ios

#endif
