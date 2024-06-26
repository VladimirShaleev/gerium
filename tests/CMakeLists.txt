enable_testing()

find_package(GTest CONFIG REQUIRED)

file(GLOB_RECURSE GERIUM_TESTS "*.hpp" "*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX tests FILES ${GERIUM_TESTS})

add_executable(gerium-tests ${GERIUM_TESTS})
target_link_libraries(gerium-tests PRIVATE gerium GTest::gtest GTest::gtest_main GTest::gmock)

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
