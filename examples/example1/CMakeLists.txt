file(GLOB GERIUM_EXAMPLE
    "*.hpp" "*.cpp"
    "services/*.hpp" "services/*.cpp"
    "passes/*.hpp" "passes/*.cpp"
    "events/*.hpp" "events/*.cpp"
    "components/*.hpp" "components/*.cpp")

file(GLOB GERIUM_GRAPH_RESOURCES "frame-graphs/*.yaml")
file(GLOB GERIUM_TECH_RESOURCES "techniques/*.yaml")
file(GLOB GERIUM_MODEL_RESOURCES "models/*.gltf")

set(LIST_IDS "")
macro(gen_ids)
    cmake_parse_arguments(gen_ids_args "" "" "RESOURCES;PREFIX" ${ARGN})
    if(NOT gen_ids_args_RESOURCES)
        message(FATAL_ERROR "RESOURCES not present")
    endif()
    if(NOT gen_ids_args_PREFIX)
        message(FATAL_ERROR "PREFIX not present")
    endif()
    foreach(resource IN LISTS gen_ids_args_RESOURCES)
        get_filename_component(NAME ${resource} NAME_WE)
        string(TOUPPER ${NAME} UPPER_NAME)
        set(LIST_IDS "${LIST_IDS}constexpr entt::hashed_string ${gen_ids_args_PREFIX}_${UPPER_NAME}_ID = \"${NAME}\";\n")
    endforeach()
endmacro()

gen_ids(RESOURCES ${GERIUM_GRAPH_RESOURCES} PREFIX GRAPH)
gen_ids(RESOURCES ${GERIUM_TECH_RESOURCES} PREFIX TECH)
gen_ids(RESOURCES ${GERIUM_MODEL_RESOURCES} PREFIX MODEL)
configure_file(Resources.hpp.in Resources.hpp @ONLY)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX example FILES ${GERIUM_EXAMPLE})

find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(meshoptimizer CONFIG REQUIRED)
find_package(CapnProto CONFIG REQUIRED)
find_package(reflectcpp CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

if(ANDROID)
    add_library(example1 SHARED ${GERIUM_EXAMPLE})
else()
    add_executable(example1 ${GERIUM_EXAMPLE})
endif()
target_link_libraries(example1 PRIVATE gerium::gerium)
target_link_libraries(example1 PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(example1 PRIVATE EnTT::EnTT)
target_link_libraries(example1 PRIVATE glm::glm-header-only)
target_link_libraries(example1 PRIVATE magic_enum::magic_enum)
target_link_libraries(example1 PRIVATE meshoptimizer::meshoptimizer)
target_link_libraries(example1 PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(example1 PRIVATE reflectcpp::reflectcpp)
target_link_libraries(example1 PRIVATE assimp::assimp)
target_include_directories(example1 PRIVATE ${Stb_INCLUDE_DIR})
target_include_directories(example1 PRIVATE ${WYHASH_INCLUDE_DIRS})
target_include_directories(example1 PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_compile_features(example1 PRIVATE cxx_std_20)
set_target_properties(example1 PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(example1 PUBLIC /GR-)
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

function(copy_dir target src dst)
    if(ANDROID)
        set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/../../../../../assets")
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

copy_dir(example1 ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ shaders)
copy_dir(example1 ${CMAKE_CURRENT_SOURCE_DIR}/techniques/ techniques)
copy_dir(example1 ${CMAKE_CURRENT_SOURCE_DIR}/frame-graphs/ frame-graphs)
copy_dir(example1 ${CMAKE_CURRENT_SOURCE_DIR}/models/ models)
