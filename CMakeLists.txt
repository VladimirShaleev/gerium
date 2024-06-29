cmake_minimum_required(VERSION 3.16)
include(cmake/version.cmake)

option(GERIUM_USE_VCPKG "Use the vcpkg package manager to resolve dependencies." ON)
option(GERIUM_INSTALL "Generate installation target" ON)
option(GERIUM_MSVC_DYNAMIC_RUNTIME "Link dynamic runtime library instead of static" OFF)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

if(GERIUM_USE_VCPKG)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    else()
        include(FetchContent)
        FetchContent_Declare(
          vcpkg
          GIT_REPOSITORY https://github.com/microsoft/vcpkg/
          GIT_TAG 2024.06.15)
        FetchContent_MakeAvailable(vcpkg)
        set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    endif()
endif()

project(gerium VERSION ${GERIUM_VERSION} LANGUAGES CXX)

find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

include(CMakeDependentOption)
include(CTest)

cmake_dependent_option(GERIUM_BUILD_TESTS "Build tests" ON
    "BUILD_TESTING;CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)
cmake_dependent_option(GERIUM_BUILD_SAMPLE "Build sample" ON
    "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

file(GLOB_RECURSE GERIUM_INCLUDE "include/*.h")
file(GLOB GERIUM_SOURCES "sources/*.hpp" "sources/*.cpp" "sources/Vulkan/*.hpp" "sources/Vulkan/*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_INCLUDE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_SOURCES})

if(WIN32)
    file(GLOB GERIUM_WINDOWS_SOURCES 
        "sources/Windows/*.hpp" "sources/Windows/*.cpp" 
        "sources/Vulkan/Windows/*.hpp" "sources/Vulkan/Windows/*.cpp")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_WINDOWS_SOURCES})
    list(APPEND GERIUM_SOURCES ${GERIUM_WINDOWS_SOURCES})
elseif(IOS)
elseif(APPLE)
    file(GLOB GERIUM_MACOS_SOURCES 
        "sources/MacOS/*.hpp" "sources/MacOS/*.cpp" "sources/MacOS/*.mm")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_MACOS_SOURCES})
    list(APPEND GERIUM_SOURCES ${GERIUM_MACOS_SOURCES})
endif()

add_library(gerium ${GERIUM_INCLUDE} ${GERIUM_SOURCES})
add_library(gerium::gerium ALIAS gerium)

target_link_libraries(gerium PRIVATE Vulkan::Headers)
target_link_libraries(gerium PRIVATE GPUOpen::VulkanMemoryAllocator)

target_include_directories(gerium PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_precompile_headers(gerium PRIVATE "sources/Gerium.hpp")
target_compile_features(gerium PRIVATE cxx_std_20)
set_target_properties(gerium PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
    PUBLIC_HEADER "${GERIUM_INCLUDE}")
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

if(WIN32)
    target_compile_definitions(gerium PUBLIC GERIUM_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(gerium PUBLIC GERIUM_PLATFORM_MAC_OS)
elseif(UNIX)
    target_compile_definitions(gerium PUBLIC GERIUM_PLATFORM_UNIX)
elseif(ANDROID)
    target_compile_definitions(gerium PUBLIC GERIUM_PLATFORM_ANDROID)
elseif(IOS)
    target_compile_definitions(gerium PUBLIC GERIUM_PLATFORM_IOS)
elseif(EMSCRIPTEN)
    target_compile_definitions(gerium PUBLIC GERIUM_PLATFORM_WEB)
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
    target_link_libraries(gerium PRIVATE "-framework Foundation")
elseif(NOT APPLE)
    target_compile_options(gerium PUBLIC -fno-rtti)
endif()

if(GERIUM_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(GERIUM_BUILD_SAMPLE)
    add_subdirectory(example)
endif()
