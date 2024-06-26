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
   GERIUM_APPLICATION_MODE_NONE_BIT       = 0,
   GERIUM_APPLICATION_MODE_FULLSCREEN_BIT = 1,
   GERIUM_APPLICATION_MODE_RESIZABLE_BIT  = 2,
   GERIUM_APPLICATION_MODE_TOPMOST        = 4,
   GERIUM_APPLICATION_MODE_MAX_ENUM       = 0x7FFFFFFF
} gerium_application_mode_flags_t;
GERIUM_FLAGS(gerium_application_mode_flags_t)

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

gerium_public gerium_result_t
gerium_application_run(gerium_application_t application);

gerium_public void
gerium_application_exit(gerium_application_t application);

GERIUM_END

#endif
