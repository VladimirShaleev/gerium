cmake_minimum_required(VERSION 3.16)
include(FetchContent)
include(cmake/version.cmake)
include(cmake/vcpkg_android.cmake)

option(GERIUM_INSTALL "Generate installation target" ON)
option(GERIUM_MSVC_DYNAMIC_RUNTIME "Link dynamic runtime library instead of static" OFF)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
else()
    FetchContent_Declare(
        vcpkg
        GIT_REPOSITORY https://github.com/microsoft/vcpkg/
        GIT_TAG 2024.06.15)
    FetchContent_MakeAvailable(vcpkg)
    set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

if(APPLE)
    FetchContent_Declare(
        moltenVK
        URL https://github.com/KhronosGroup/MoltenVK/releases/download/v1.2.9/MoltenVK-all.tar
        URL_HASH MD5=1219a051a60c0cf71021983a91b82c0a
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(moltenVK)
    set(MOLTEN_VK_SRC "${moltenvk_SOURCE_DIR}/MoltenVK" CACHE STRING "MoltenVK dir")
endif()

FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/v1.90.9.zip
    URL_HASH MD5=845a67241e6acd3b689ef8fc7c327b7a
)
FetchContent_MakeAvailable(imgui)
set(IMGUI_INCLUDE_DIRS "${imgui_SOURCE_DIR}" "${imgui_SOURCE_DIR}/backends")
set(IMGUI_SOURCES
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"
    "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp")

project(gerium VERSION ${GERIUM_VERSION} LANGUAGES CXX)

find_package(mimalloc CONFIG REQUIRED)
find_package(CMakeRC CONFIG REQUIRED)
find_package(ctre CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)
find_package(unofficial-spirv-reflect CONFIG REQUIRED)
find_package(marl CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_path(WYHASH_INCLUDE_DIRS "wyhash.h")
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_package(X11 REQUIRED)
endif()

include(CMakeDependentOption)

cmake_dependent_option(GERIUM_BUILD_SAMPLE "Build sample" ON
    "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

file(GLOB GERIUM_INCLUDE "include/*.h")
file(GLOB GERIUM_SOURCES "sources/*.hpp" "sources/*.cpp" "sources/Vulkan/*.hpp" "sources/Vulkan/*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_INCLUDE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_SOURCES})

file(GLOB GERIUM_RESOURCES "resources/*")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_RESOURCES})

cmrc_add_resource_library(
    gerium-resources ALIAS gerium::resources 
        NAMESPACE gerium::resources
        TYPE STATIC
        ${GERIUM_RESOURCES}
)
target_compile_features(gerium-resources PRIVATE cxx_std_20)
set_target_properties(gerium-resources PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS OFF)
if(MSVC)
    target_compile_options(gerium-resources PUBLIC /GR-)
    if(GERIUM_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(gerium-resources PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(gerium-resources PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

if(WIN32)
    file(GLOB GERIUM_WINDOWS_SOURCES 
        "sources/Windows/*.hpp" "sources/Windows/*.cpp" 
        "sources/Vulkan/Windows/*.hpp" "sources/Vulkan/Windows/*.cpp")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_WINDOWS_SOURCES})
    list(APPEND GERIUM_SOURCES ${GERIUM_WINDOWS_SOURCES})
    list(APPEND IMGUI_SOURCES "${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp")
elseif(ANDROID)
    file(GLOB GERIUM_ANDROID_SOURCES
        "sources/Unix/*.hpp" "sources/Unix/*.cpp"
        "sources/Android/*.hpp" "sources/Android/*.cpp"
        "sources/Vulkan/Android/*.hpp" "sources/Vulkan/Android/*.cpp")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_ANDROID_SOURCES})
    list(APPEND GERIUM_SOURCES ${GERIUM_ANDROID_SOURCES})
    list(APPEND IMGUI_SOURCES "${imgui_SOURCE_DIR}/backends/imgui_impl_android.cpp")
elseif(IOS)
elseif(APPLE)
    enable_language(OBJCXX)
    file(GLOB GERIUM_MACOS_SOURCES 
        "sources/Unix/*.hpp" "sources/Unix/*.cpp"
        "sources/MacOS/*.hpp" "sources/MacOS/*.cpp" "sources/MacOS/*.mm"
        "sources/Vulkan/MacOS/*.hpp" "sources/Vulkan/MacOS/*.cpp" "sources/Vulkan/MacOS/*.mm")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_MACOS_SOURCES})
    list(APPEND GERIUM_SOURCES ${GERIUM_MACOS_SOURCES})
    list(APPEND IMGUI_SOURCES "${imgui_SOURCE_DIR}/backends/imgui_impl_osx.mm")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    file(GLOB GERIUM_LINUX_SOURCES 
        "sources/Unix/*.hpp" "sources/Unix/*.cpp"
        "sources/Linux/*.hpp" "sources/Linux/*.cpp" 
        "sources/Vulkan/Linux/*.hpp" "sources/Vulkan/Linux/*.cpp")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_LINUX_SOURCES})
    list(APPEND GERIUM_SOURCES ${GERIUM_LINUX_SOURCES})
endif()

add_library(gerium ${GERIUM_INCLUDE} ${GERIUM_SOURCES})
add_library(gerium::gerium ALIAS gerium)

target_link_libraries(gerium PRIVATE $<IF:$<TARGET_EXISTS:mimalloc-static>,mimalloc-static,mimalloc>)
target_link_libraries(gerium PRIVATE gerium::resources)
target_link_libraries(gerium PRIVATE ctre::ctre)
target_link_libraries(gerium PRIVATE absl::flat_hash_map)
target_link_libraries(gerium PRIVATE Vulkan::Headers)
target_link_libraries(gerium PRIVATE GPUOpen::VulkanMemoryAllocator)
target_link_libraries(gerium PRIVATE unofficial::shaderc::shaderc)
target_link_libraries(gerium PRIVATE unofficial::spirv-reflect)
target_link_libraries(gerium PRIVATE marl::marl)
target_link_libraries(gerium PRIVATE $<$<PLATFORM_ID:Linux>:X11::X11>)  # libx11-dev, libxi-dev, libxinerama-dev, libxrandr-dev
target_link_libraries(gerium PRIVATE $<$<PLATFORM_ID:Linux>:X11::X11_xcb>)
target_include_directories(gerium PRIVATE ${WYHASH_INCLUDE_DIRS})
target_include_directories(gerium PRIVATE ${IMGUI_INCLUDE_DIRS})
target_include_directories(gerium PRIVATE ${Stb_INCLUDE_DIR})
target_sources(gerium PRIVATE ${IMGUI_SOURCES})

target_include_directories(gerium PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(gerium PUBLIC
    $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>)

target_precompile_headers(gerium PRIVATE "sources/Gerium.hpp")
target_compile_features(gerium PRIVATE cxx_std_20)
set_target_properties(gerium PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS OFF)
if(BUILD_SHARED_LIBS)
    set_target_properties(gerium PROPERTIES 
        VERSION ${PROJECT_VERSION} 
        SOVERSION ${PROJECT_VERSION_MAJOR})
    set_target_properties(gerium PROPERTIES
        CXX_VISIBILITY_PRESET hidden 
        VISIBILITY_INLINES_HIDDEN ON)
else()
    target_compile_definitions(gerium PUBLIC GERIUM_STATIC_BUILD)
endif()

if(ANDROID)
    enable_language(C)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
    add_library(gerium-native-app-glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
    add_library(gerium::native-app-glue ALIAS gerium-native-app-glue)
    target_include_directories(gerium-native-app-glue PUBLIC
        $<BUILD_INTERFACE:${ANDROID_NDK}/sources/android/native_app_glue/>
        $<INSTALL_INTERFACE:include>)
    target_link_libraries(gerium-native-app-glue PRIVATE android log)
    target_compile_features(gerium-native-app-glue PRIVATE cxx_std_20)
    set_target_properties(gerium-native-app-glue PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON
        WINDOWS_EXPORT_ALL_SYMBOLS OFF
        PUBLIC_HEADER ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.h)

    target_link_libraries(gerium PRIVATE gerium::native-app-glue)
endif()

if(MSVC)
    target_compile_options(gerium PUBLIC /GR-)
    if(GERIUM_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(gerium PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(gerium PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
elseif(APPLE)
    enable_language(OBJCXX)
    # file(GLOB_RECURSE GERIUM_SOURCES_OBJC "sources/*.mm")
    # source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_SOURCES_OBJC})
    # target_sources(gerium PRIVATE ${GERIUM_SOURCES_OBJC})
    # target_link_libraries(gerium PRIVATE "${MOLTEN_VK_SRC}/static/MoltenVK.xcframework/macos-arm64_x86_64/libMoltenVK.a")
    target_link_libraries(gerium PRIVATE "${MOLTEN_VK_SRC}/static/MoltenVK.xcframework/macos-arm64_x86_64/libMoltenVK.a")
    target_link_libraries(gerium PRIVATE "-framework Foundation" "-framework QuartzCore" "-framework AppKit" "-framework IOKit" "-framework MetalKit" "-framework Metal" "-framework IOSurface" "-framework GameController")
elseif(NOT APPLE)
    target_compile_options(gerium PUBLIC -fno-rtti)
    if (ANDROID)
        target_link_libraries(gerium PRIVATE android log)
    endif()
endif()

if(GERIUM_BUILD_SAMPLE)
    add_subdirectory(example)
endif()
