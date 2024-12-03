#ifndef GERIUM_LINUX_LINUX_FILE_HPP
#define GERIUM_LINUX_LINUX_FILE_HPP

#include "../Unix/UnixFile.hpp"

namespace gerium::linux {

class LinuxFile final : public nix::UnixFile {
public:
    explicit LinuxFile(gerium_uint64_t size);
    LinuxFile(gerium_utf8_t path, gerium_uint64_t size);
    LinuxFile(gerium_utf8_t path, bool readOnly);

    ~LinuxFile() override;

private:
    std::string createTempFile();

    FILE* _file{};
};

} // namespace gerium::macos

#endif
