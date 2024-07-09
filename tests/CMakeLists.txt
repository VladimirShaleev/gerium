enable_testing()

find_package(GTest CONFIG REQUIRED)

file(GLOB GERIUM_TESTS "*.hpp" "*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX tests FILES ${GERIUM_TESTS})

add_executable(gerium-tests ${GERIUM_TESTS} ${GERIUM_INCLUDE} ${GERIUM_SOURCES})
target_link_libraries(gerium-tests PRIVATE gerium GTest::gtest GTest::gtest_main GTest::gmock)
target_link_libraries(gerium-tests PRIVATE $<IF:$<TARGET_EXISTS:mimalloc-static>,mimalloc-static,mimalloc>)
target_link_libraries(gerium-tests PRIVATE ctre::ctre)
target_link_libraries(gerium-tests PRIVATE absl::flat_hash_map)
target_link_libraries(gerium-tests PRIVATE Vulkan::Headers)
target_link_libraries(gerium-tests PRIVATE GPUOpen::VulkanMemoryAllocator)
target_include_directories(gerium-tests PRIVATE ${WYHASH_INCLUDE_DIRS})

target_precompile_headers(gerium-tests PRIVATE "${PROJECT_SOURCE_DIR}/sources/Gerium.hpp")
target_compile_features(gerium-tests PRIVATE cxx_std_20)
set_target_properties(gerium-tests PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

target_include_directories(gerium-tests PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/sources>
    $<INSTALL_INTERFACE:sources>)

if(MSVC)
    if(GERIUM_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(gerium-tests PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(gerium-tests PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

include(GoogleTest)
gtest_discover_tests(gerium-tests)
