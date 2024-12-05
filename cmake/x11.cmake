if(NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
    return()
endif()

include(ExternalProject)

FetchContent_Declare(
    x11
    GIT_REPOSITORY https://github.com/hexops/x11-headers.git
    GIT_TAG 29aefb525d5c08b05b0351e34b1623854a138c21
)
FetchContent_MakeAvailable(x11)
