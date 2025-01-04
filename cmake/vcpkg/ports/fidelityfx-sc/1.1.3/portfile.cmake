vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO GPUOpen-LibrariesAndSDKs/FidelityFX-SDK
    REF "v${VERSION}"
    SHA512 3e689a34ce83941107dc791fbe1633fc9b35c832fec6c7cd9527f73cc8d20be608f549b1b846db8297b3327848b4226222e49324e05bbba362ac44ce7646a3b1
    HEAD_REF main
    PATCHES
        fidelityfx-sc.patch
)

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}/sdk/tools/ffx_shader_compiler")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

vcpkg_cmake_install()
vcpkg_copy_tools(TOOL_NAMES FidelityFX_SC AUTO_CLEAN)
vcpkg_cmake_config_fixup(PACKAGE_NAME FidelityFX_SC CONFIG_PATH share/cmake/FidelityFX_SC)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
