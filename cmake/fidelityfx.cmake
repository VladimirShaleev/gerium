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
    CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE};-DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET};-DFFX_API_BACKEND=VK_X64;-DFFX_FSR3=ON;-DFFX_SSSR=ON;-DFFX_ALL=ON;-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>;-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
)

ExternalProject_Get_property(FidelityFX SOURCE_DIR)

macro(linkFfxLib libName)
    if(WIN32)
        target_link_libraries(gerium PRIVATE ${SOURCE_DIR}/sdk/bin/ffx_sdk/ffx_${libName}_native$<$<CONFIG:Debug>:d>.lib)
    else()
        target_link_libraries(gerium PRIVATE ${SOURCE_DIR}/sdk/bin/ffx_sdk/${CMAKE_FIND_LIBRARY_PREFIXES}ffx_${libName}_native$<$<CONFIG:Debug>:d>.a)
    endif()
endmacro()

add_dependencies(gerium FidelityFX-build)
linkFfxLib(backend_vk)
linkFfxLib(brixelizer)
linkFfxLib(brixelizergi)
linkFfxLib(cacao)
target_include_directories(gerium PUBLIC ${SOURCE_DIR}/sdk/include)
