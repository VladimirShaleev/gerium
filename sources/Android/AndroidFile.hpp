#ifndef GERIUM_ANDROID_ANDROID_FILE_HPP
#define GERIUM_ANDROID_ANDROID_FILE_HPP

#include "../Unix/UnixFile.hpp"
#include "AndroidApplication.hpp"

namespace gerium::android {

class AndroidFile final : public unix::UnixFile {
public:
    explicit AndroidFile(gerium_uint64_t size);
    AndroidFile(gerium_utf8_t path, gerium_uint64_t size);
    AndroidFile(gerium_utf8_t path, bool readOnly);

    static gerium_utf8_t getCacheDirFromContext();

private:
    static std::string getTempFile();

    static void initialize();

    static jclass _fileClass;
    static jmethodID _getAbsolutePath;
    static jmethodID _createTempFile;
    static jmethodID _deleteOnExit;
    static std::string _cacheDir;
};

} // namespace gerium::android

#endif
