#ifndef GERIUM_ANDROID_ANDROID_FILE_HPP
#define GERIUM_ANDROID_ANDROID_FILE_HPP

#include "../File.hpp"
#include "AndroidApplication.hpp"

namespace gerium::android {

class AndroidFile final : public File {
public:
    explicit AndroidFile(gerium_uint64_t size);
    AndroidFile(gerium_utf8_t path, gerium_uint64_t size);
    AndroidFile(gerium_utf8_t path, bool readOnly);

    ~AndroidFile();

    static gerium_utf8_t getCacheDir();
    static gerium_utf8_t getAppDir() noexcept;
    static std::string getTempFile();

    static bool existsFile(gerium_utf8_t path);
    static bool existsDir(gerium_utf8_t path);
    static void deleteFile(gerium_utf8_t path);

private:
    gerium_uint64_t onGetSize() noexcept override;
    void onSeek(gerium_uint64_t offset, gerium_file_seek_t seek) noexcept override;
    void onWrite(gerium_cdata_t data, gerium_uint32_t size) override;
    gerium_uint32_t onRead(gerium_data_t data, gerium_uint32_t size) noexcept override;
    gerium_data_t onMap() noexcept override;

    static void initialize();

    static jclass _fileClass;
    static jmethodID _getAbsolutePath;
    static jmethodID _createTempFile;
    static jmethodID _deleteOnExit;
    static std::string _cacheDir;

    int _file;
    gerium_data_t _data;
    bool _readOnly;
};

} // namespace gerium::android

#endif
