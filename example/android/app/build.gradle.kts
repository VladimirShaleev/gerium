plugins {
    id("com.android.application")
}

android {
    namespace = "com.github.vladimirshaleev.gerium"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.github.vladimirshaleev.gerium"
        minSdk = 21
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"
        @Suppress("UnstableApiUsage") externalNativeBuild {
            cmake {
                arguments += "-DGERIUM_BUILD_TESTS=OFF"
                arguments += "-DANDROID_TOOLCHAIN=clang"
                arguments += "-DANDROID_STL=c++_static"
                arguments += "-DCMAKE_TOOLCHAIN_FILE="
                cppFlags += "-std=c++20"
            }
        }
    }
    buildTypes {
        release {
            isMinifyEnabled = true
        }
    }
    externalNativeBuild {
        cmake {
            path = file("../../../CMakeLists.txt")
            version = "3.22.1"
        }
    }

    sourceSets["main"].assets {
        srcDir(".cxx/assets")
    }
}
