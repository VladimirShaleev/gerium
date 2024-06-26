cmake_minimum_required(VERSION 3.16)
include(cmake/version.cmake)

option(GERIUM_USE_VCPKG "Use the vcpkg package manager to resolve dependencies." ON)
option(GERIUM_INSTALL "Generate installation target" ON)
option(GERIUM_MSVC_DYNAMIC_RUNTIME "Link dynamic runtime library instead of static" OFF)

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

include(CMakeDependentOption)
include(CTest)

cmake_dependent_option(GERIUM_BUILD_TESTS "Build tests" ON
  "BUILD_TESTING;CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

file(GLOB_RECURSE GERIUM_SOURCES "sources/*.hpp" "sources/*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_SOURCES})

add_library(gerium ${GERIUM_SOURCES})

# target_precompile_headers(gerium PRIVATE "sources/common.hpp")
target_compile_features(gerium PRIVATE cxx_std_20)
set_target_properties(gerium PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)

if(MSVC)
  target_compile_options(gerium PUBLIC /GR-)
  target_compile_options(gerium PRIVATE -DNOMINMAX)
  if(GERIUM_MSVC_DYNAMIC_RUNTIME)
    set_target_properties(gerium PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  else()
    set_target_properties(gerium PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()
elseif(APPLE)
  enable_language(OBJCXX)
  file(GLOB_RECURSE GERIUM_SOURCES_OBJC "sources/*.mm")
  source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_SOURCES_OBJC})
  target_sources(vision-flow PRIVATE ${GERIUM_SOURCES_OBJC})
  # target_link_libraries(vision-flow PRIVATE "${MOLTEN_VK_SRC}/static/MoltenVK.xcframework/macos-arm64_x86_64/libMoltenVK.a")
  target_link_libraries(vision-flow PRIVATE "-framework IOSurface")
elseif(NOT APPLE)
  target_compile_options(vision-flow PUBLIC -fno-rtti)
endif()

if(GERIUM_BUILD_TESTS)
  add_subdirectory(tests)
endif()
