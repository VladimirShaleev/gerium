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

typedef struct _gerium_application* gerium_application_t;

typedef enum
{
   GERIUM_RESULT_SUCCESS                = 0,
   GERIUM_RESULT_UNKNOWN_ERROR          = 1,
   GERIUM_RESULT_OUT_OF_MEMORY          = 2,
   GERIUM_RESULT_NOT_IMPLEMENTED        = 3,
   GERIUM_RESULT_FEATURE_NOT_SUPPORTED  = 4,
   GERIUM_RESULT_INVALID_ARGUMENT       = 5,
   GERIUM_RESULT_NO_DISPLAY             = 6,
   GERIUM_RESULT_APPLICATION_RUNNING    = 7,
   GERIUM_RESULT_APPLICATION_TERMINATED = 8,
   GERIUM_RESULT_MAX_ENUM               = 0x7FFFFFFF
} gerium_result_t;

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
   GERIUM_APPLICATION_STATE_UNKNOWN      = 0,
   GERIUM_APPLICATION_STATE_CREATE       = 1,
   GERIUM_APPLICATION_STATE_DESTROY      = 2,
   GERIUM_APPLICATION_STATE_INITIALIZE   = 3,
   GERIUM_APPLICATION_STATE_UNINITIALIZE = 4,
   GERIUM_APPLICATION_STATE_GOT_FOCUS    = 5,
   GERIUM_APPLICATION_STATE_LOST_FOCUS   = 6,
   GERIUM_APPLICATION_STATE_VISIBLE      = 7,
   GERIUM_APPLICATION_STATE_INVISIBLE    = 8,
   GERIUM_APPLICATION_STATE_NORMAL       = 9,
   GERIUM_APPLICATION_STATE_MINIMIZE     = 10,
   GERIUM_APPLICATION_STATE_MAXIMIZE     = 11,
   GERIUM_APPLICATION_STATE_FULLSCREEN   = 12,
   GERIUM_APPLICATION_STATE_RESIZE       = 13,
   GERIUM_APPLICATION_STATE_MAX_ENUM     = 0x7FFFFFFF
} gerium_application_state_t;

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

gerium_public void
gerium_application_fullscreen(gerium_application_t application,
                              gerium_bool_t fullscreen,
                              const gerium_display_mode_t* mode);

gerium_public gerium_result_t
gerium_application_run(gerium_application_t application);

gerium_public void
gerium_application_exit(gerium_application_t application);

GERIUM_END

#endif
