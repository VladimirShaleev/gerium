#include "Win32File.hpp"
#include "Unicode.hpp"

#include <shlobj.h>

namespace gerium::windows {

Win32File::Win32File(gerium_uint32_t size) : Win32File(getTempFileName().c_str(), size) {
}

Win32File::Win32File(gerium_utf8_t path, gerium_uint32_t size) :
    _file(INVALID_HANDLE_VALUE),
    _map(INVALID_HANDLE_VALUE),
    _data(nullptr),
    _readOnly(false) {
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

Win32File::Win32File(gerium_utf8_t path, bool readOnly) :
    _file(INVALID_HANDLE_VALUE),
    _map(INVALID_HANDLE_VALUE),
    _data(nullptr),
    _readOnly(readOnly) {
    const auto file = gerium::windows::wideString(path);

    auto flags = GENERIC_READ;

    if (!readOnly) {
        flags |= GENERIC_WRITE;
    }

    _file = CreateFileW(file.c_str(), flags, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (_file == INVALID_HANDLE_VALUE) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }
}

Win32File::~Win32File() {
    if (_data) {
        UnmapViewOfFile(_data);
    }

    if (_map != INVALID_HANDLE_VALUE) {
        CloseHandle(_map);
    }

    if (_file != INVALID_HANDLE_VALUE) {
        CloseHandle(_file);
    }
}

gerium_uint32_t Win32File::onGetSize() noexcept {
    return {};
}

void Win32File::onWrite(gerium_cdata_t data, gerium_uint32_t size) {
}

gerium_uint32_t Win32File::onRead(gerium_data_t data, gerium_uint32_t size) noexcept {
    return {};
}

gerium_data_t Win32File::onMap() noexcept {
    if (!_data) {
        _map = CreateFileMappingW(_file, NULL, _readOnly ? PAGE_READONLY : PAGE_READWRITE, 0, 0, nullptr);
        if (_map == INVALID_HANDLE_VALUE) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }

        DWORD flags = FILE_MAP_READ;
        if (!_readOnly) {
            flags |= FILE_MAP_WRITE;
        }

        _data = MapViewOfFile(_map, flags, 0, 0, 0);

        if (!_data) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }
    }
    return _data;
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

gerium_result_t gerium_file_open(gerium_utf8_t path, gerium_bool_t read_only, gerium_file_t* file) {
    using namespace gerium;
    using namespace gerium::windows;
    return Object::create<Win32File>(*file, path, read_only != 0);
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
