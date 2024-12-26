include (FetchContent)
get_filename_component(ASSETS "../assets" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_BASE_DIR ${ASSETS})

FetchContent_Declare(
    gerium_assets
    GIT_REPOSITORY https://github.com/VladimirShaleev/gerium-assets.git
    GIT_TAG        c1db61685029566c3f062a35550841f1f294ad26
    GIT_PROGRESS   TRUE
)

FetchContent_GetProperties(gerium_assets)
if(NOT gerium_assets_POPULATED)
    FetchContent_Populate(gerium_assets)
endif()

include(CMakeDependentOption)
cmake_dependent_option(GERIUM_BUILD_CLUSTER_PACK "Build cluster pack" ON "NOT CMAKE_CROSSCOMPILING" OFF)

file(GLOB GERIUM_EXAMPLE "*.hpp" "*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX example FILES ${GERIUM_EXAMPLE})

find_package(yaml-cpp CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(meshoptimizer CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_executable(example2 ${GERIUM_EXAMPLE})
target_link_libraries(example2 PRIVATE gerium::gerium)
target_link_libraries(example2 PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(example2 PRIVATE glm::glm-header-only)
target_link_libraries(example2 PRIVATE magic_enum::magic_enum)
target_link_libraries(example2 PRIVATE meshoptimizer::meshoptimizer)
target_include_directories(example2 PRIVATE ${WYHASH_INCLUDE_DIRS})

target_compile_features(example2 PRIVATE cxx_std_20)
set_target_properties(example2 PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(example2 PUBLIC /GR-)
    target_compile_options(example2 PRIVATE -DNOMINMAX)
    if(GERIUM_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(example2 PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(example2 PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
    set_target_properties(example2 PROPERTIES WIN32_EXECUTABLE ON)
elseif(ANDROID)
elseif(APPLE)
    set_target_properties(example2 PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_RPATH ON
        MACOSX_BUNDLE_BUNDLE_NAME "example2"
        MACOSX_BUNDLE_BUNDLE_VERSION 1
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.vladimirshaleev.gerium"
        MACOSX_BUNDLE_INFO_STRING "Example 2"
        MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path/Libraries"
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.github.vladimirshaleev.gerium")
elseif(NOT APPLE)
    target_compile_options(example2 PUBLIC -fno-rtti)
endif()

if(GERIUM_BUILD_CLUSTER_PACK)
    find_package(argparse CONFIG REQUIRED)
    find_package(assimp CONFIG REQUIRED)
    find_package(gli CONFIG REQUIRED)
    find_package(Ktx CONFIG REQUIRED)
    add_executable(cluster-pack cluster-pack/ClusterPack.cpp)
    target_compile_features(cluster-pack PRIVATE cxx_std_20)
    target_link_libraries(cluster-pack PRIVATE argparse::argparse)
    target_link_libraries(cluster-pack PRIVATE assimp::assimp)
    target_link_libraries(cluster-pack PRIVATE glm::glm-header-only)
    target_link_libraries(cluster-pack PRIVATE meshoptimizer::meshoptimizer)
    target_link_libraries(cluster-pack PRIVATE gli)
    target_link_libraries(cluster-pack PRIVATE KTX::ktx KTX::astcenc-avx2-static)
    target_include_directories(cluster-pack PRIVATE ${Stb_INCLUDE_DIR})
    if(MSVC)
        target_compile_options(cluster-pack PUBLIC /GR-)
        target_compile_options(cluster-pack PRIVATE -DNOMINMAX)
        if(GERIUM_MSVC_DYNAMIC_RUNTIME)
            set_target_properties(cluster-pack PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        else()
            set_target_properties(cluster-pack PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
    endif()
endif()

function(copy_dir target src dst)
    if(ANDROID)
        set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/../../../../assets")
    elseif(APPLE)
        set(OUTPUT_PATH "$<TARGET_FILE_DIR:${target}>/../Resources")
    else()
        set(OUTPUT_PATH "$<TARGET_FILE_DIR:${target}>")
    endif()
    if(CMAKE_VERSION VERSION_LESS "3.26.0")
        set(COPY_ARG copy_directory)
    else()
        set(COPY_ARG copy_directory_if_different)
    endif()
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E ${COPY_ARG}
            "${src}"
            "${OUTPUT_PATH}/${dst}"
        COMMENT "Copying files")
endfunction()

copy_dir(example2 ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ shaders)
copy_dir(example2 ${CMAKE_CURRENT_SOURCE_DIR}/techniques/ techniques)
copy_dir(example2 ${CMAKE_CURRENT_SOURCE_DIR}/frame-graphs/ frame-graphs)
copy_dir(example2 ${gerium_assets_SOURCE_DIR}/textures textures)
copy_dir(example2 ${gerium_assets_SOURCE_DIR}/models models)
