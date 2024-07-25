#ifndef GERIUM_MAC_OS_MAC_OS_FILE_HPP
#define GERIUM_MAC_OS_MAC_OS_FILE_HPP

#include "../File.hpp"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

namespace gerium::macos {

class MacOSFile final : public File {
public:
    explicit MacOSFile(gerium_uint64_t size);
    MacOSFile(gerium_utf8_t path, gerium_uint64_t size);
    MacOSFile(gerium_utf8_t path, bool readOnly);

    ~MacOSFile() override;

    static std::string getPath(NSSearchPathDirectory directory) noexcept;
    static bool exists(gerium_utf8_t path, bool isDir) noexcept;

private:
    gerium_uint64_t onGetSize() noexcept override;
    void onSeek(gerium_uint64_t offset, gerium_file_seek_t seek) noexcept override;
    void onWrite(gerium_cdata_t data, gerium_uint32_t size) override;
    gerium_uint32_t onRead(gerium_data_t data, gerium_uint32_t size) noexcept override;
    gerium_data_t onMap() noexcept override;
};

} // namespace gerium::macos

#endif
