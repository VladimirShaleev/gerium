@file:Suppress("UnstableApiUsage")

import java.util.Locale

plugins {
    id("com.android.application")
}

android {
    namespace = "com.github.vladimirshaleev.gerium"
    compileSdk = 36
    ndkVersion = "28.0.12674087"

    defaultConfig {
        applicationId = "com.github.vladimirshaleev.gerium"
        minSdk = 21
        targetSdk = 36
        versionCode = 1
        versionName = "1.0"
        externalNativeBuild {
            cmake {
                arguments += "-DANDROID_TOOLCHAIN=clang"
                arguments += "-DANDROID_STL=c++_static"
                arguments += "-DCMAKE_TOOLCHAIN_FILE="
                cppFlags += "-std=c++20"
                cppFlags += "-ffp-contract=off"
            }
        }
        ndk {
            abiFilters += listOf("x86_64", "arm64-v8a", "armeabi-v7a")
        }
    }
    buildTypes {
        release {
            isMinifyEnabled = true
        }
    }
    externalNativeBuild {
        cmake {
            path = file("../../../../CMakeLists.txt")
            version = "3.31.1"
        }
    }
    applicationVariants.configureEach {
        val variant =
            name.replaceFirstChar { if (it.isLowerCase()) it.titlecase(Locale.getDefault()) else it.toString() }
        tasks["merge${variant}Assets"].dependsOn("merge${variant}NativeLibs")
    }
    sourceSets["main"].assets.srcDir(".cxx/assets")
}
