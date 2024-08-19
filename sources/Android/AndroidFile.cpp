#include "AndroidFile.hpp"

namespace gerium {

namespace android {

AndroidFile::AndroidFile(gerium_uint64_t size) : AndroidFile(getTempFile().c_str(), size) {
}

AndroidFile::AndroidFile(gerium_utf8_t path, gerium_uint64_t size) : unix::UnixFile(path, size) {
}

AndroidFile::AndroidFile(gerium_utf8_t path, bool readOnly) : unix::UnixFile(copyFromAssets(path, readOnly), readOnly) {
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

gerium_utf8_t AndroidFile::copyFromAssets(gerium_utf8_t path, bool readOnly) {
    if (readOnly && !existsFile(path)) {
        std::filesystem::path appDir = std::filesystem::path(AndroidFile::getAppDir());
        auto relative                = std::filesystem::relative(path, appDir).string();
        auto asset                   = AAssetManager_open(
            AndroidApplication::instance()->activity->assetManager, relative.c_str(), AASSET_MODE_BUFFER);
        if (asset) {
            createDirs(path);
            char buffer[1024];
            int read  = 0;
            auto file = fopen(path, "wb");
            while ((read = AAsset_read(asset, buffer, 1024)) > 0) {
                fwrite(buffer, read, 1, file);
            }
            fclose(file);
            AAsset_close(asset);
        }
    }
    return path;
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
