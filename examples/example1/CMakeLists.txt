include(${PROJECT_SOURCE_DIR}/cmake/assets.cmake)

file(GLOB GERIUM_EXAMPLE
    "*.hpp" "*.cpp"
    "utils/*.hpp" "utils/*.cpp"
    "shaders/common/*.h"
    "services/*.hpp" "services/*.cpp"
    "passes/*.hpp" "passes/*.cpp"
    "events/*.hpp" "events/*.cpp"
    "components/*.hpp" "components/*.cpp"
    "develop/*.hpp" "develop/*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${GERIUM_EXAMPLE})

file(GLOB GERIUM_GRAPH_RESOURCES "frame-graphs/*.yaml")
file(GLOB GERIUM_TECH_RESOURCES "techniques/*.yaml")
file(GLOB GERIUM_MODEL_RESOURCES "${gerium_assets_SOURCE_DIR}/sample1/*.gltf")
file(GLOB GERIUM_TEXTURES_RESOURCES
    "${gerium_assets_SOURCE_DIR}/sample1/textures/*.png"
    "${gerium_assets_SOURCE_DIR}/sample1/textures/*.ktx"
    "${gerium_assets_SOURCE_DIR}/sample1/textures/*.ktx2")

set(LIST_IDS "")
set(ARRAY_IDS "")
string(TIMESTAMP GEN_DATETIME %Y-%m-%dT%H:%M:%SZ UTC)
macro(gen_ids)
    cmake_parse_arguments(gen_ids_args "RELATIVE_PATH" "" "RESOURCES;PREFIX" ${ARGN})
    if(NOT gen_ids_args_RESOURCES)
        message(FATAL_ERROR "RESOURCES not present")
    endif()
    if(NOT gen_ids_args_PREFIX)
        message(FATAL_ERROR "PREFIX not present")
    endif()
    string(TOLOWER ${gen_ids_args_PREFIX} ARRAY_NAME)
    set(ARRAY_IDS "constexpr entt::hashed_string ${ARRAY_NAME}Ids[] = {")
    foreach(resource IN LISTS gen_ids_args_RESOURCES)
        get_filename_component(NAME ${resource} NAME_WE)
        string(REPLACE "-" "_" NAME "${NAME}") 
        string(TOUPPER ${NAME} UPPER_NAME)
        set(NAME_ID "${gen_ids_args_PREFIX}_${UPPER_NAME}_ID")
        if (gen_ids_args_RELATIVE_PATH)
            file(RELATIVE_PATH NAME ${CMAKE_CURRENT_SOURCE_DIR} ${resource})
        endif()
        set(LIST_IDS "${LIST_IDS}constexpr entt::hashed_string ${NAME_ID} = \"${NAME}\";\n")
        set(ARRAY_IDS "${ARRAY_IDS}\n    ${NAME_ID},")
    endforeach()
    set(ARRAY_IDS "${ARRAY_IDS}\n};")
endmacro()

gen_ids(RESOURCES ${GERIUM_GRAPH_RESOURCES} PREFIX GRAPH)
gen_ids(RESOURCES ${GERIUM_TECH_RESOURCES} PREFIX TECH)
set(TECH_IDS "${ARRAY_IDS}")
gen_ids(RESOURCES ${GERIUM_MODEL_RESOURCES} PREFIX MODEL)
set(MODEL_IDS "${ARRAY_IDS}")
gen_ids(RESOURCES ${GERIUM_TEXTURES_RESOURCES} PREFIX TEX RELATIVE_PATH)
set(TEX_IDS "${ARRAY_IDS}")
configure_file(utils/Resources.hpp.in Resources.hpp @ONLY)

find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(meshoptimizer CONFIG REQUIRED)
find_package(CapnProto CONFIG REQUIRED)
find_package(reflectcpp CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Jolt CONFIG REQUIRED)

if(ANDROID)
    add_library(example1 SHARED ${GERIUM_EXAMPLE})
else()
    add_executable(example1 ${GERIUM_EXAMPLE})
endif()
target_link_libraries(example1 PRIVATE gerium::gerium)
target_link_libraries(example1 PRIVATE imgui::imgui)
target_link_libraries(example1 PRIVATE imguizmo::imguizmo)
target_link_libraries(example1 PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(example1 PRIVATE EnTT::EnTT)
target_link_libraries(example1 PRIVATE glm::glm-header-only)
target_link_libraries(example1 PRIVATE magic_enum::magic_enum)
target_link_libraries(example1 PRIVATE meshoptimizer::meshoptimizer)
target_link_libraries(example1 PRIVATE reflectcpp::reflectcpp)
target_link_libraries(example1 PRIVATE stduuid)
target_link_libraries(example1 PRIVATE assimp::assimp)
target_link_libraries(example1 PRIVATE Jolt::Jolt)
target_include_directories(example1 PRIVATE ${Stb_INCLUDE_DIR})
target_include_directories(example1 PRIVATE ${WYHASH_INCLUDE_DIRS})
target_include_directories(example1 PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_precompile_headers(example1 PRIVATE "Common.hpp")
target_compile_features(example1 PRIVATE cxx_std_20)
set_target_properties(example1 PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(example1 PUBLIC /GR-)
    target_compile_options(example1 PRIVATE /bigobj)
    target_compile_options(example1 PRIVATE -DNOMINMAX)
    if(GERIUM_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(example1 PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(example1 PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
    set_target_properties(example1 PROPERTIES WIN32_EXECUTABLE ON)
elseif(ANDROID)
elseif(APPLE)
    set_target_properties(example1 PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_RPATH ON
        MACOSX_BUNDLE_BUNDLE_NAME "example1"
        MACOSX_BUNDLE_BUNDLE_VERSION 1
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.vladimirshaleev.gerium.example1"
        MACOSX_BUNDLE_INFO_STRING "Example 1"
        MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path/Libraries"
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.github.vladimirshaleev.gerium.example1")
elseif(NOT APPLE)
    target_compile_options(example1 PUBLIC -fno-rtti)
endif()

copy_dir(example1 ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ shaders)
copy_dir(example1 ${CMAKE_CURRENT_SOURCE_DIR}/techniques/ techniques)
copy_dir(example1 ${CMAKE_CURRENT_SOURCE_DIR}/frame-graphs/ frame-graphs)
copy_dir(example1 ${gerium_assets_SOURCE_DIR}/sample1/ models)
