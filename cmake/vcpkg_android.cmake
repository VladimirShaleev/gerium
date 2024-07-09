if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    string(REPLACE "\\" "/" CMAKE_ANDROID_NDK ${CMAKE_ANDROID_NDK})

    set(ENV{ANDROID_NDK_HOME} "${CMAKE_ANDROID_NDK}")
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_ANDROID_NDK}/build/cmake/android.toolchain.cmake")

    if (ANDROID_ABI MATCHES "arm64-v8a")
        set(VCPKG_TARGET_TRIPLET "arm64-android" CACHE STRING "" FORCE)
    elseif(ANDROID_ABI MATCHES "armeabi-v7a")
        set(VCPKG_TARGET_TRIPLET "arm-neon-android" CACHE STRING "" FORCE)
    elseif(ANDROID_ABI MATCHES "x86_64")
        set(VCPKG_TARGET_TRIPLET "x64-android" CACHE STRING "" FORCE)
    elseif(ANDROID_ABI MATCHES "x86")
        set(VCPKG_TARGET_TRIPLET "x86-android" CACHE STRING "" FORCE)
    else()
        message(FATAL_ERROR "Possible ABIs are: arm64-v8a, armeabi-v7a, x64-android, x86-android")
    endif()
    message("VCPKG_TARGET_TRIPLET was set to ${VCPKG_TARGET_TRIPLET}")
endif()