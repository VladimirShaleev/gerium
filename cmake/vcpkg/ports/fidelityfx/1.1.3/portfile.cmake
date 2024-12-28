vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO GPUOpen-LibrariesAndSDKs/FidelityFX-SDK
    REF "v${VERSION}"
    SHA512 3e689a34ce83941107dc791fbe1633fc9b35c832fec6c7cd9527f73cc8d20be608f549b1b846db8297b3327848b4226222e49324e05bbba362ac44ce7646a3b1
    HEAD_REF main
    PATCHES
        fidelityfx.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}/sdk"
    OPTIONS
        -DFFX_API_BACKEND=VK_X64
        -DFFX_FSR3=ON
        -DFFX_SSSR=ON
        -DFFX_ALL=ON
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()

vcpkg_cmake_config_fixup(PACKAGE_NAME FidelityFX CONFIG_PATH share/cmake/FidelityFX)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
