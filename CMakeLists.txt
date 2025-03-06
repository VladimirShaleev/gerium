cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0135 NEW)

include(cmake/version.cmake)
include(cmake/vcpkg.cmake)
include(cmake/x11.cmake)
include(cmake/properties.cmake)
include(CMakeDependentOption)

project(gerium VERSION ${GERIUM_VERSION} LANGUAGES CXX)

option(GERIUM_INSTALL "Generate installation target" ON)
option(GERIUM_MSVC_DYNAMIC_RUNTIME "Link dynamic runtime library instead of static" OFF)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
cmake_dependent_option(GERIUM_BUILD_SAMPLES "Build samples" ON
    "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

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
find_package(imgui CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_package(FidelityFX CONFIG REQUIRED)
find_path(RAPIDHASH_INCLUDE_DIRS "rapidhash.h")

file(GLOB GERIUM_INCLUDE "include/*.h" "include/*.hpp")
file(GLOB GERIUM_SOURCES "sources/*.hpp" "sources/*.cpp" "sources/Vulkan/*.hpp" "sources/Vulkan/*.cpp")
file(GLOB GERIUM_RESOURCES "resources/*")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_INCLUDE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_RESOURCES})

if(WIN32)
    file(GLOB GERIUM_WINDOWS_SOURCES 
        "sources/Windows/*.hpp" "sources/Windows/*.cpp" 
        "sources/Vulkan/Windows/*.hpp" "sources/Vulkan/Windows/*.cpp")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_WINDOWS_SOURCES})
    list(APPEND GERIUM_SOURCES ${GERIUM_WINDOWS_SOURCES})
elseif(ANDROID)
    file(GLOB GERIUM_ANDROID_SOURCES
        "sources/Unix/*.hpp" "sources/Unix/*.cpp"
        "sources/Android/*.hpp" "sources/Android/*.cpp"
        "sources/Vulkan/Android/*.hpp" "sources/Vulkan/Android/*.cpp")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_ANDROID_SOURCES})
    list(APPEND GERIUM_SOURCES ${GERIUM_ANDROID_SOURCES})
elseif(IOS)
elseif(APPLE)
    enable_language(OBJCXX)
    file(GLOB GERIUM_MACOS_SOURCES 
        "sources/Unix/*.hpp" "sources/Unix/*.cpp"
        "sources/MacOS/*.hpp" "sources/MacOS/*.cpp" "sources/MacOS/*.mm"
        "sources/Vulkan/MacOS/*.hpp" "sources/Vulkan/MacOS/*.cpp" "sources/Vulkan/MacOS/*.mm")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_MACOS_SOURCES})
    list(APPEND GERIUM_SOURCES ${GERIUM_MACOS_SOURCES})
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    file(GLOB GERIUM_LINUX_SOURCES 
        "sources/Unix/*.hpp" "sources/Unix/*.cpp"
        "sources/Linux/*.hpp" "sources/Linux/*.cpp" 
        "sources/Vulkan/Linux/*.hpp" "sources/Vulkan/Linux/*.cpp")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_LINUX_SOURCES})
    list(APPEND GERIUM_SOURCES ${GERIUM_LINUX_SOURCES})
endif()

cmrc_add_resource_library(
    gerium-resources ALIAS gerium::resources 
        NAMESPACE gerium::resources
        TYPE STATIC
        ${GERIUM_RESOURCES})
set_target_default_properties(gerium-resources)

add_library(gerium ${GERIUM_INCLUDE} ${GERIUM_SOURCES})
add_library(gerium::gerium ALIAS gerium)

target_include_directories(gerium PRIVATE ${RAPIDHASH_INCLUDE_DIRS})
target_include_directories(gerium PRIVATE ${Stb_INCLUDE_DIR})
target_include_directories(gerium PRIVATE $<$<PLATFORM_ID:Linux>:${x11_SOURCE_DIR}>)
target_link_libraries(gerium PRIVATE $<IF:$<TARGET_EXISTS:mimalloc-static>,mimalloc-static,mimalloc>)
target_link_libraries(gerium PRIVATE gerium::resources)
target_link_libraries(gerium PRIVATE ctre::ctre)
target_link_libraries(gerium PRIVATE absl::flat_hash_map)
target_link_libraries(gerium PRIVATE Vulkan::Headers)
target_link_libraries(gerium PRIVATE GPUOpen::VulkanMemoryAllocator)
target_link_libraries(gerium PRIVATE unofficial::shaderc::shaderc)
target_link_libraries(gerium PRIVATE unofficial::spirv-reflect)
target_link_libraries(gerium PRIVATE marl::marl)
target_link_libraries(gerium PRIVATE imgui::imgui)
target_link_libraries(gerium PRIVATE KTX::ktx $<$<PLATFORM_ID:Windows>:KTX::astcenc-avx2-static>)
target_link_libraries(gerium PRIVATE
    fidelityfx::backend_vk
    fidelityfx::blur
    fidelityfx::breadcrumbs
    fidelityfx::brixelizer
    fidelityfx::brixelizergi
    fidelityfx::cacao
    fidelityfx::cas
    fidelityfx::classifier
    fidelityfx::denoiser
    fidelityfx::dof
    fidelityfx::fsr3upscaler
    fidelityfx::opticalflow
    fidelityfx::fsr1
    fidelityfx::fsr2
    fidelityfx::fsr3
    fidelityfx::lens
    fidelityfx::lpm
    fidelityfx::parallelsort
    fidelityfx::spd
    fidelityfx::sssr
    fidelityfx::vrs)

target_include_directories(gerium PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

set_target_default_properties(gerium)
target_precompile_headers(gerium PRIVATE "sources/Gerium.hpp")

if(MSVC)
elseif(APPLE)
    enable_language(OBJCXX)
    include(FetchContent)
    FetchContent_Declare(
        moltenVK
        URL https://github.com/KhronosGroup/MoltenVK/releases/download/v1.2.11-artifacts/MoltenVK-all.tar
        URL_HASH MD5=d88d601e6db0966930b94a18266584e4
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(moltenVK)
    set(MOLTEN_VK_SRC "${moltenvk_SOURCE_DIR}/MoltenVK" CACHE STRING "MoltenVK dir")
    target_link_libraries(gerium PRIVATE "${MOLTEN_VK_SRC}/static/MoltenVK.xcframework/macos-arm64_x86_64/libMoltenVK.a")
    target_link_libraries(gerium PRIVATE
        "-framework Foundation"
        "-framework QuartzCore"
        "-framework AppKit"
        "-framework IOKit"
        "-framework MetalKit"
        "-framework Metal"
        "-framework IOSurface"
        "-framework GameController")
elseif(ANDROID)
    enable_language(C)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
    set(GERIUM_NATIVE_APP_GLUE_DIR "${ANDROID_NDK}/sources/android/native_app_glue")
    add_library(gerium-native-app-glue STATIC ${GERIUM_NATIVE_APP_GLUE_DIR}/android_native_app_glue.c)
    add_library(gerium::native-app-glue ALIAS gerium-native-app-glue)
    set_target_default_properties(gerium-native-app-glue)
    set_target_properties(gerium-native-app-glue PROPERTIES PUBLIC_HEADER ${GERIUM_NATIVE_APP_GLUE_DIR}/android_native_app_glue.h)
    target_link_libraries(gerium-native-app-glue PRIVATE android log)
    target_include_directories(gerium-native-app-glue PUBLIC
        $<BUILD_INTERFACE:${GERIUM_NATIVE_APP_GLUE_DIR}/>
        $<INSTALL_INTERFACE:include>)
    target_link_libraries(gerium PRIVATE gerium::native-app-glue android log)
endif()

if(GERIUM_BUILD_SAMPLES)
    add_subdirectory(examples/example1)
    add_subdirectory(examples/example2)
endif()
