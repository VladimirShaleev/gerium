if(NOT GERIUM_FIDELITY_FX OR NOT WIN32)
    return()
endif()

find_package(Vulkan REQUIRED)

include(ExternalProject)

ExternalProject_Add(
    FidelityFX
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK
    GIT_TAG tags/v1.1.3
    STEP_TARGETS build
    EXCLUDE_FROM_ALL TRUE
    SOURCE_SUBDIR sdk
    UPDATE_DISCONNECTED TRUE
    PATCH_COMMAND git apply --ignore-space-change --ignore-whitespace "${CMAKE_CURRENT_LIST_DIR}/fidelityfx.patch"
    CMAKE_ARGS "-DFFX_API_BACKEND=VK_X64;-DFFX_FSR3=ON;-DFFX_SSSR=ON;-DFFX_ALL=ON;-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

ExternalProject_Get_property(FidelityFX SOURCE_DIR)

add_dependencies(gerium FidelityFX-build)
target_link_libraries(gerium PRIVATE Vulkan::Vulkan)
target_link_libraries(gerium PRIVATE ${SOURCE_DIR}/sdk/bin/ffx_sdk/ffx_backend_vk_x64$<$<CONFIG:Debug>:d>.lib)
target_link_libraries(gerium PRIVATE ${SOURCE_DIR}/sdk/bin/ffx_sdk/ffx_brixelizer_x64$<$<CONFIG:Debug>:d>.lib)
target_link_libraries(gerium PRIVATE ${SOURCE_DIR}/sdk/bin/ffx_sdk/ffx_brixelizergi_x64$<$<CONFIG:Debug>:d>.lib)
target_link_libraries(gerium PRIVATE ${SOURCE_DIR}/sdk/bin/ffx_sdk/ffx_cacao_x64$<$<CONFIG:Debug>:d>.lib)
target_compile_definitions(gerium PUBLIC GERIUM_FIDELITY_FX)
target_include_directories(gerium PUBLIC ${SOURCE_DIR}/sdk/include)
