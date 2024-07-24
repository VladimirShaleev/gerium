#include "Win32File.hpp"
#include "Unicode.hpp"

#include <shlobj.h>

namespace gerium::windows {

Win32File::Win32File(gerium_uint32_t size) : Win32File(getTempFileName().c_str(), size) {
}

Win32File::Win32File(gerium_utf8_t path, gerium_uint32_t size) : _file(INVALID_HANDLE_VALUE) {
    const auto file = gerium::windows::wideString(path);

    _file =
        CreateFileW(file.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (_file == INVALID_HANDLE_VALUE) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }

    if (size) {
        if (SetFilePointer(_file, size, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }
        if (!SetEndOfFile(_file)) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }
        SetFilePointer(_file, 0, 0, FILE_BEGIN);
    }
}

Win32File::~Win32File() {
    if (_file != INVALID_HANDLE_VALUE) {
        CloseHandle(_file);
    }
}

std::string Win32File::getTempFileName() {
    wchar_t tempPath[MAX_PATH];
    wchar_t tempFile[MAX_PATH];

    auto result = GetTempPathW(MAX_PATH, tempPath);
    if (result > MAX_PATH || (result == 0)) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }

    result = GetTempFileNameW(tempPath, nullptr, 0, tempFile);
    if (result == 0) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }

    return utf8String(tempFile);
}

} // namespace gerium::windows

gerium_utf8_t gerium_file_get_cache_dir(void) {
    static std::string dir;

    if (dir.empty()) {
        wchar_t path[MAX_PATH];
        HRESULT result = SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);

        if (FAILED(result)) {
            assert(!"Get local app data dir failed");
            return "";
        }

        dir = gerium::windows::utf8String(path);
    }

    return dir.c_str();
}

gerium_utf8_t gerium_file_get_app_dir(void) {
    static std::string dir;

    if (dir.empty()) {
        wchar_t path[512];
        HRESULT result = GetModuleFileNameW(GetModuleHandleW(nullptr), path, 512);

        if (FAILED(result)) {
            assert(!"Get local app data dir failed");
            return "";
        }

        dir = gerium::windows::utf8String(std::filesystem::path(path).parent_path().wstring());
    }

    return dir.c_str();
}

gerium_bool_t gerium_file_exists_file(gerium_utf8_t path) {
    const auto file  = gerium::windows::wideString(path);
    const auto attrs = GetFileAttributesW(file.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY;
}

gerium_bool_t gerium_file_exists_dir(gerium_utf8_t path) {
    const auto file  = gerium::windows::wideString(path);
    const auto attrs = GetFileAttributesW(file.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}

void gerium_file_delete_file(gerium_utf8_t path) {
    const auto file = gerium::windows::wideString(path);
    DeleteFileW(file.c_str());
}

gerium_result_t gerium_file_create(gerium_utf8_t path, gerium_uint32_t size, gerium_file_t* file) {
    using namespace gerium;
    using namespace gerium::windows;
    return Object::create<Win32File>(*file, path, size);
}

gerium_result_t gerium_file_create_temp(gerium_uint32_t size, gerium_file_t* file) {
    using namespace gerium;
    using namespace gerium::windows;
    return Object::create<Win32File>(*file, size);
}
