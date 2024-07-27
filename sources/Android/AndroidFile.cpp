#include "AndroidFile.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace gerium {

namespace android {

AndroidFile::AndroidFile(gerium_uint64_t size) : AndroidFile(getTempFile().c_str(), size) {
}

AndroidFile::AndroidFile(gerium_utf8_t path, gerium_uint64_t size) : File(false), _file(-1), _data(nullptr) {
    const auto dirs = std::filesystem::path(path).parent_path().string();
    ::mkdir(dirs.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    _file = ::open(path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (_file < 0) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }

    if (size) {
        if (::fallocate64(_file, 0, 0, (off64_t) size) != 0) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }
    }
}

AndroidFile::AndroidFile(gerium_utf8_t path, bool readOnly) : File(readOnly), _file(-1), _data(nullptr) {
    const auto dirs = std::filesystem::path(path).parent_path().string();
    ::mkdir(dirs.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    _file = ::open(path, (readOnly ? O_RDONLY : O_RDWR) | O_CREAT, S_IRUSR | (readOnly ? 0 : S_IWUSR));

    if (_file < 0) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }
}

AndroidFile::~AndroidFile() {
    if (_data) {
        ::munmap(_data, onGetSize());
    }

    if (_file >= 0) {
        ::close(_file);
    }
}

gerium_utf8_t AndroidFile::getCacheDirFromContext() {
    initialize();
    return _cacheDir.c_str();
}

std::string AndroidFile::getTempFile() {
    initialize();

    auto vm = AndroidApplication::instance()->activity->vm;
    if (JNIEnv * env; vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        auto prefix = env->NewStringUTF("gerium");

        auto file = env->CallStaticObjectMethod(_fileClass, _createTempFile, prefix, (jstring) nullptr);
        auto dir  = (jstring) env->CallObjectMethod(file, _getAbsolutePath);

        auto dirLength = env->GetStringUTFLength(dir);
        std::string name;
        name.resize(dirLength + 1);
        env->GetStringUTFRegion(dir, 0, dirLength, name.data());

        env->CallVoidMethod(file, _deleteOnExit);

        env->DeleteLocalRef(dir);
        env->DeleteLocalRef(file);
        env->DeleteLocalRef(prefix);

        return name;
    }
    return "";
}

gerium_uint64_t AndroidFile::onGetSize() noexcept {
    struct stat64 stat {};

    ::fstat64(_file, &stat);
    return (gerium_uint64_t) stat.st_size;
}

void AndroidFile::onSeek(gerium_uint64_t offset, gerium_file_seek_t seek) noexcept {
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
    ::lseek64(_file, off64_t(offset), seek);
}

void AndroidFile::onWrite(gerium_cdata_t data, gerium_uint32_t size) {
    if (::write(_file, (const void*) data, (size_t) size) < 0) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }
}

gerium_uint32_t AndroidFile::onRead(gerium_data_t data, gerium_uint32_t size) noexcept {
    return (gerium_uint32_t)::read(_file, (void*) data, (size_t) size);
}

gerium_data_t AndroidFile::onMap() noexcept {
    if (!_data) {
        _data = ::mmap64(nullptr,
                         onGetSize(),
                         PROT_READ | (isReadOnly() ? 0 : PROT_WRITE),
                         isReadOnly() ? MAP_PRIVATE : MAP_SHARED,
                         _file,
                         0);
    }
    return _data;
}

void AndroidFile::initialize() {
    if (_cacheDir.empty()) {
        auto app      = AndroidApplication::instance();
        auto activity = app->activity;

        if (JNIEnv * env; activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
            auto activityClazz = activity->clazz;

            auto contextClass = env->FindClass("android/content/Context");
            auto getCacheDir  = env->GetMethodID(contextClass, "getCacheDir", "()Ljava/io/File;");

            _fileClass       = (jclass) env->NewGlobalRef(env->FindClass("java/io/File"));
            _getAbsolutePath = env->GetMethodID(_fileClass, "getAbsolutePath", "()Ljava/lang/String;");
            _createTempFile  = env->GetStaticMethodID(
                _fileClass, "createTempFile", "(Ljava/lang/String;Ljava/lang/String;)Ljava/io/File;");
            _deleteOnExit = env->GetMethodID(_fileClass, "deleteOnExit", "()V");

            auto file = env->CallObjectMethod(activityClazz, getCacheDir);

            auto dir       = (jstring) env->CallObjectMethod(file, _getAbsolutePath);
            auto dirLength = env->GetStringUTFLength(dir);
            _cacheDir.resize(dirLength + 1);
            env->GetStringUTFRegion(dir, 0, dirLength, _cacheDir.data());

            env->DeleteLocalRef(dir);
            env->DeleteLocalRef(file);
            env->DeleteLocalRef(contextClass);

            activity->vm->DetachCurrentThread();
        }
    }
}

jclass AndroidFile::_fileClass = nullptr;

jmethodID AndroidFile::_getAbsolutePath = nullptr;

jmethodID AndroidFile::_createTempFile = nullptr;

jmethodID AndroidFile::_deleteOnExit = nullptr;

std::string AndroidFile::_cacheDir;

} // namespace android

gerium_utf8_t File::getCacheDir() noexcept {
    return android::AndroidFile::getCacheDirFromContext();
}

gerium_utf8_t File::getAppDir() noexcept {
    return android::AndroidApplication::instance()->activity->internalDataPath;
}

bool File::existsFile(gerium_utf8_t path) noexcept {
    std::filesystem::path file = path;
    return std::filesystem::exists(file) && !std::filesystem::is_directory(file);
}

bool File::existsDir(gerium_utf8_t path) noexcept {
    std::filesystem::path dir = path;
    return std::filesystem::exists(dir) && std::filesystem::is_directory(dir);
}

void File::deleteFile(gerium_utf8_t path) noexcept {
    if (existsFile(path)) {
        std::filesystem::remove(path);
    }
}

} // namespace gerium

using namespace gerium;
using namespace gerium::android;

gerium_result_t gerium_file_open(gerium_utf8_t path, gerium_bool_t read_only, gerium_file_t* file) {
    return Object::create<AndroidFile>(*file, path, read_only != 0);
}

gerium_result_t gerium_file_create(gerium_utf8_t path, gerium_uint64_t size, gerium_file_t* file) {
    return Object::create<AndroidFile>(*file, path, size);
}

gerium_result_t gerium_file_create_temp(gerium_uint64_t size, gerium_file_t* file) {
    return Object::create<AndroidFile>(*file, size);
}
