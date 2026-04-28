if(NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
    return()
endif()

include(ExternalProject)

FetchContent_Declare(
    x11
    GIT_REPOSITORY https://github.com/LordMZTE/x11-headers.git
    GIT_TAG 040c368534fc28c20ac01ef810e9a693271d2ff0
)
FetchContent_MakeAvailable(x11)
