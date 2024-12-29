import java.util.Locale

plugins {
    id("com.android.application")
}

android {
    namespace = "com.github.vladimirshaleev.gerium"
    compileSdk = 34
    ndkVersion = "27.1.12297006"

    defaultConfig {
        applicationId = "com.github.vladimirshaleev.gerium"
        minSdk = 21
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"
        externalNativeBuild {
            cmake {
                arguments += "-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON"
                arguments += "-DANDROID_TOOLCHAIN=clang"
                arguments += "-DANDROID_STL=c++_static"
                arguments += "-DCMAKE_TOOLCHAIN_FILE="
                cppFlags += "-std=c++20"
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
