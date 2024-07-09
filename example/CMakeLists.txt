file(GLOB GERIUM_EXAMPLE "*.hpp" "*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX example FILES ${GERIUM_EXAMPLE})

if(ANDROID)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
    add_library(cpp-example SHARED ${GERIUM_EXAMPLE})
    target_link_libraries(cpp-example PRIVATE gerium::native-app-glue)
else()
    add_executable(cpp-example ${GERIUM_EXAMPLE})
endif()
target_link_libraries(cpp-example PRIVATE gerium::gerium)

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
elseif(APPLE)
elseif(NOT APPLE)
    target_compile_options(cpp-example PUBLIC -fno-rtti)
endif()
