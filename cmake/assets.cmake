include(FetchContent)
get_filename_component(ASSETS "${CMAKE_BINARY_DIR}/../" ABSOLUTE)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL ASSETS)
    set(ASSETS "${CMAKE_BINARY_DIR}")
endif()
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_BASE_DIR ${ASSETS})

FetchContent_Declare(
    gerium_assets
    GIT_REPOSITORY https://github.com/VladimirShaleev/gerium-assets.git
    GIT_TAG        8e85b438020a122b8b9a7e3d62f51ad139d24467
    GIT_PROGRESS   TRUE
)

FetchContent_GetProperties(gerium_assets)
if(NOT gerium_assets_POPULATED)
    FetchContent_Populate(gerium_assets)
endif()

function(copy_dir target src dst)
    if(ANDROID)
        set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/../../../../assets")
    elseif(APPLE)
        set(OUTPUT_PATH "$<TARGET_FILE_DIR:${target}>/../Resources")
    else()
        set(OUTPUT_PATH "$<TARGET_FILE_DIR:${target}>")
    endif()
    if(CMAKE_VERSION VERSION_LESS "3.26.0")
        set(COPY_ARG copy_directory)
    else()
        set(COPY_ARG copy_directory_if_different)
    endif()
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E ${COPY_ARG}
            "${src}"
            "${OUTPUT_PATH}/${dst}"
        COMMENT "Copying files")
endfunction()
