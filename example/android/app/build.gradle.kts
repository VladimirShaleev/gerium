import java.util.Locale

plugins {
    id("com.android.application")
}

android {
    namespace = "com.github.vladimirshaleev.gerium"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.github.vladimirshaleev.gerium"
        minSdk = 21
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"
        externalNativeBuild {
            cmake {
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
        }
    }
    applicationVariants.configureEach {
        val variant =
            name.replaceFirstChar { if (it.isLowerCase()) it.titlecase(Locale.getDefault()) else it.toString() }
        tasks["merge${variant}Assets"].dependsOn("merge${variant}NativeLibs")
    }
    sourceSets["main"].assets.srcDir(".cxx/assets")
}
