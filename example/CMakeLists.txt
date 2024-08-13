file(GLOB GERIUM_EXAMPLE "*.hpp" "*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX example FILES ${GERIUM_EXAMPLE})

if(ANDROID)
    add_library(cpp-example SHARED ${GERIUM_EXAMPLE})
else()
    add_executable(cpp-example ${GERIUM_EXAMPLE})
endif()
target_link_libraries(cpp-example PRIVATE gerium::gerium)
target_link_libraries(cpp-example PRIVATE glm::glm-header-only)
target_link_libraries(cpp-example PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(cpp-example PRIVATE EnTT::EnTT)

target_compile_features(cpp-example PRIVATE cxx_std_20)
set_target_properties(cpp-example PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(cpp-example PUBLIC /GR-)
    target_compile_options(cpp-example PRIVATE -DNOMINMAX)
    if(GERIUM_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(cpp-example PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(cpp-example PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
    set_target_properties(cpp-example PROPERTIES
        WIN32_EXECUTABLE ON)
elseif(APPLE)
    set_target_properties(cpp-example PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_RPATH ON
        MACOSX_BUNDLE_BUNDLE_NAME "cpp-example"
        MACOSX_BUNDLE_BUNDLE_VERSION 1
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.vladimirshaleev.gerium"
        MACOSX_BUNDLE_INFO_STRING "Cpp Example"
        MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path/Libraries"
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES)
elseif(NOT APPLE)
    target_compile_options(cpp-example PUBLIC -fno-rtti)
endif()
