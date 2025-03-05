function(set_target_default_properties TARGET)
    set(options STATIC SHARED)
    cmake_parse_arguments(ARGS "${options}" "" "" ${ARGN})

    if(ARGS_STATIC)
        set(IS_STATIC YES)
    elseif(ARGS_SHARED)
        set(IS_STATIC NO)
    else()
        if(BUILD_SHARED_LIBS)
            set(IS_STATIC NO)
        else()
            set(IS_STATIC YES)
        endif()
    endif()

    target_compile_features(${TARGET} PRIVATE cxx_std_20)
    set_target_properties(${TARGET} PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON
        WINDOWS_EXPORT_ALL_SYMBOLS OFF)
    if(IS_STATIC)
        target_compile_definitions(${TARGET} PUBLIC GERIUM_STATIC_BUILD)
    else()
        set_target_properties(${TARGET} PROPERTIES 
            VERSION ${PROJECT_VERSION} 
            SOVERSION ${PROJECT_VERSION_MAJOR})
        set_target_properties(${TARGET} PROPERTIES
            CXX_VISIBILITY_PRESET hidden 
            VISIBILITY_INLINES_HIDDEN ON)
    endif()

    if(MSVC)
        target_compile_options(${TARGET} PUBLIC /GR-)
        if(GERIUM_MSVC_DYNAMIC_RUNTIME)
            set_target_properties(${TARGET} PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        else()
            set_target_properties(${TARGET} PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
    elseif(NOT APPLE)
        target_compile_options(${TARGET} PUBLIC -fno-rtti)
    endif()
endfunction()
