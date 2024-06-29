/**
 * \file      gerium-core.h
 * \brief     gerium API Core
 * \author    Vladimir Shaleev
 * \copyright MIT License
 */

#ifndef GERIUM_CORE_H
#define GERIUM_CORE_H

#include "gerium-version.h"
#include "gerium-platform.h"

GERIUM_BEGIN

typedef struct _gerium_logger* gerium_logger_t;

typedef struct _gerium_application* gerium_application_t;

typedef struct _gerium_renderer* gerium_renderer_t;

typedef enum
{
   GERIUM_RESULT_SUCCESS                           = 0,
   GERIUM_RESULT_ERROR_UNKNOWN                     = 1,
   GERIUM_RESULT_ERROR_OUT_OF_MEMORY               = 2,
   GERIUM_RESULT_ERROR_NOT_IMPLEMENTED             = 3,
   GERIUM_RESULT_ERROR_FROM_CALLBACK               = 4,
   GERIUM_RESULT_ERROR_FEATURE_NOT_SUPPORTED       = 5,
   GERIUM_RESULT_ERROR_INVALID_ARGUMENT            = 6,
   GERIUM_RESULT_ERROR_NO_DISPLAY                  = 7,
   GERIUM_RESULT_ERROR_APPLICATION_ALREADY_RUNNING = 8,
   GERIUM_RESULT_ERROR_APPLICATION_NOT_RUNNING     = 9,
   GERIUM_RESULT_ERROR_APPLICATION_TERMINATED      = 10,
   GERIUM_RESULT_ERROR_CHANGE_DISPLAY_MODE         = 11,
   GERIUM_RESULT_MAX_ENUM                          = 0x7FFFFFFF
} gerium_result_t;

typedef enum {
   GERIUM_LOGGER_LEVEL_VERBOSE  = 0,
   GERIUM_LOGGER_LEVEL_DEBUG    = 1,
   GERIUM_LOGGER_LEVEL_INFO     = 2,
   GERIUM_LOGGER_LEVEL_WARNING  = 3,
   GERIUM_LOGGER_LEVEL_ERROR    = 4,
   GERIUM_LOGGER_LEVEL_FATAL    = 5,
   GERIUM_LOGGER_LEVEL_MAX_ENUM = 0x7FFFFFFF
} gerium_logger_level_t;

typedef enum
{
   GERIUM_RUNTIME_PLATFORM_UNKNOWN  = 0,
   GERIUM_RUNTIME_PLATFORM_ANDROID  = 1,
   GERIUM_RUNTIME_PLATFORM_IOS      = 2,
   GERIUM_RUNTIME_PLATFORM_WEB      = 3,
   GERIUM_RUNTIME_PLATFORM_WINDOWS  = 4,
   GERIUM_RUNTIME_PLATFORM_LINUX    = 5,
   GERIUM_RUNTIME_PLATFORM_MAC_OS   = 6,
   GERIUM_RUNTIME_PLATFORM_MAX_ENUM = 0x7FFFFFFF
} gerium_runtime_platform_t;

typedef enum
{
   GERIUM_APPLICATION_STATE_UNKNOWN       = 0,
   GERIUM_APPLICATION_STATE_CREATE        = 1,
   GERIUM_APPLICATION_STATE_DESTROY       = 2,
   GERIUM_APPLICATION_STATE_INITIALIZE    = 3,
   GERIUM_APPLICATION_STATE_UNINITIALIZE  = 4,
   GERIUM_APPLICATION_STATE_GOT_FOCUS     = 5,
   GERIUM_APPLICATION_STATE_LOST_FOCUS    = 6,
   GERIUM_APPLICATION_STATE_VISIBLE       = 7,
   GERIUM_APPLICATION_STATE_INVISIBLE     = 8,
   GERIUM_APPLICATION_STATE_NORMAL        = 9,
   GERIUM_APPLICATION_STATE_MINIMIZE      = 10,
   GERIUM_APPLICATION_STATE_MAXIMIZE      = 11,
   GERIUM_APPLICATION_STATE_FULLSCREEN    = 12,
   GERIUM_APPLICATION_STATE_RESIZE        = 13,
   GERIUM_APPLICATION_STATE_RESIZED       = 14,
   GERIUM_APPLICATION_STATE_MAX_ENUM      = 0x7FFFFFFF
} gerium_application_state_t;

typedef enum
{
   GERIUM_APPLICATION_STYLE_NONE_BIT        = 0,
   GERIUM_APPLICATION_STYLE_RESIZABLE_BIT   = 1,
   GERIUM_APPLICATION_STYLE_MINIMIZABLE_BIT = 2,
   GERIUM_APPLICATION_STYLE_MAXIMIZABLE_BIT = 4,
   GERIUM_APPLICATION_STYLE_MAX_ENUM        = 0x7FFFFFFF
} gerium_application_style_flags_t;
GERIUM_FLAGS(gerium_application_style_flags_t)

typedef struct
{
   gerium_uint16_t width;
   gerium_uint16_t height;
   gerium_uint16_t refresh_rate;
} gerium_display_mode_t;

typedef struct
{
   gerium_utf8_t                name;
   gerium_utf8_t                gpu_name;
   gerium_utf8_t                device_name;
   gerium_uint32_t              mode_count;
   const gerium_display_mode_t* modes;
} gerium_display_info_t;

typedef gerium_bool_t
(*gerium_application_frame_func_t)(gerium_application_t application,
                                   gerium_data_t data,
                                   gerium_float32_t elapsed);

typedef gerium_bool_t
(*gerium_application_state_func_t)(gerium_application_t application,
                                   gerium_data_t data,
                                   gerium_application_state_t state);

gerium_public gerium_uint32_t
gerium_version(void);

gerium_public gerium_utf8_t
gerium_version_string(void);

gerium_public gerium_utf8_t
gerium_result_to_string(gerium_result_t result);

gerium_public gerium_result_t
gerium_logger_create(gerium_utf8_t tag,
                     gerium_logger_t* logger);

gerium_public gerium_logger_t
gerium_logger_reference(gerium_logger_t logger);

gerium_public void
gerium_logger_destroy(gerium_logger_t logger);

gerium_public gerium_logger_level_t
gerium_logger_get_level(gerium_logger_t logger);

gerium_public void
gerium_logger_set_level(gerium_logger_t logger,
                        gerium_logger_level_t level);

gerium_public void
gerium_logger_print(gerium_logger_t logger,
                    gerium_logger_level_t level,
                    gerium_utf8_t message);

gerium_public gerium_result_t
gerium_application_create(gerium_utf8_t title,
                          gerium_uint32_t width,
                          gerium_uint32_t height,
                          gerium_application_t* application);

gerium_public gerium_application_t
gerium_application_reference(gerium_application_t application);

gerium_public void
gerium_application_destroy(gerium_application_t application);

gerium_public gerium_runtime_platform_t
gerium_application_get_platform(gerium_application_t application);

gerium_public gerium_application_frame_func_t
gerium_application_get_frame_func(gerium_application_t application,
                                  gerium_data_t* data);

gerium_public void
gerium_application_set_frame_func(gerium_application_t application,
                                  gerium_application_frame_func_t callback,
                                  gerium_data_t data);

gerium_public gerium_application_state_func_t
gerium_application_get_state_func(gerium_application_t application,
                                  gerium_data_t* data);

gerium_public void
gerium_application_set_state_func(gerium_application_t application,
                                  gerium_application_state_func_t callback,
                                  gerium_data_t data);

gerium_public gerium_result_t
gerium_application_get_display_info(gerium_application_t application,
                                    gerium_uint32_t* display_count,
                                    gerium_display_info_t* displays);

gerium_public gerium_bool_t
gerium_application_is_fullscreen(gerium_application_t application);

gerium_public gerium_result_t
gerium_application_fullscreen(gerium_application_t application,
                              gerium_bool_t fullscreen,
                              const gerium_display_mode_t* mode);

gerium_public gerium_application_style_flags_t
gerium_application_get_style(gerium_application_t application);

gerium_public void
gerium_application_set_style(gerium_application_t application,
                             gerium_application_style_flags_t style);

gerium_public void
gerium_application_get_min_size(gerium_application_t application,
                                gerium_uint16_t* width,
                                gerium_uint16_t* height);

gerium_public void
gerium_application_set_min_size(gerium_application_t application,
                                gerium_uint16_t width,
                                gerium_uint16_t height);

gerium_public void
gerium_application_get_max_size(gerium_application_t application,
                                gerium_uint16_t* width,
                                gerium_uint16_t* height);

gerium_public void
gerium_application_set_max_size(gerium_application_t application,
                                gerium_uint16_t width,
                                gerium_uint16_t height);

gerium_public void
gerium_application_get_size(gerium_application_t application,
                            gerium_uint16_t* width,
                            gerium_uint16_t* height);

gerium_public void
gerium_application_set_size(gerium_application_t application,
                            gerium_uint16_t width,
                            gerium_uint16_t height);

gerium_public gerium_utf8_t
gerium_application_get_title(gerium_application_t application);

gerium_public void
gerium_application_set_title(gerium_application_t application,
                             gerium_utf8_t title);

gerium_public gerium_bool_t
gerium_application_get_background_wait(gerium_application_t application);

gerium_public void
gerium_application_set_background_wait(gerium_application_t application,
                                       gerium_bool_t enable);

gerium_public gerium_result_t
gerium_application_run(gerium_application_t application);

gerium_public void
gerium_application_exit(gerium_application_t application);

gerium_public gerium_result_t
gerium_renderer_create(gerium_application_t application,
                       gerium_renderer_t* renderer);

gerium_public gerium_renderer_t
gerium_renderer_reference(gerium_renderer_t renderer);

gerium_public void
gerium_renderer_destroy(gerium_renderer_t renderer);

GERIUM_END

#endif
