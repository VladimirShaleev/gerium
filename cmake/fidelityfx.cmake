if(NOT GERIUM_FIDELITY_FX OR NOT WIN32)
    return()
endif()

find_package(Vulkan REQUIRED)

include(ExternalProject)

ExternalProject_Add(
    FidelityFX
    URL https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK/archive/v1.1.2.zip
    URL_HASH MD5=2041985f64b87b1c2e48340cd7c83ca5
    GIT_PROGRESS TRUE
    STEP_TARGETS build
    EXCLUDE_FROM_ALL TRUE
    SOURCE_SUBDIR sdk
    CMAKE_ARGS "-DFFX_API_BACKEND=VK_X64;-DFFX_FSR3=ON;-DFFX_SSSR=ON;-DFFX_ALL=ON;-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

ExternalProject_Get_property(FidelityFX SOURCE_DIR)

add_dependencies(gerium FidelityFX-build)
target_link_libraries(gerium PRIVATE Vulkan::Vulkan)
target_link_libraries(gerium PRIVATE ${SOURCE_DIR}/sdk/bin/ffx_sdk/ffx_backend_vk_x64$<$<CONFIG:Debug>:d>.lib)
target_link_libraries(gerium PRIVATE ${SOURCE_DIR}/sdk/bin/ffx_sdk/ffx_brixelizer_x64$<$<CONFIG:Debug>:d>.lib)
target_link_libraries(gerium PRIVATE ${SOURCE_DIR}/sdk/bin/ffx_sdk/ffx_brixelizergi_x64$<$<CONFIG:Debug>:d>.lib)
target_compile_definitions(gerium PUBLIC GERIUM_FIDELITY_FX)
target_include_directories(gerium PUBLIC ${SOURCE_DIR}/sdk/include)
