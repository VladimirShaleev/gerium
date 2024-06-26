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

typedef enum
{
   GERIUM_STATE_SUCCESS               = 0,
   GERIUM_STATE_UNKNOWN_ERROR         = 1,
   GERIUM_STATE_OUT_OF_MEMORY         = 2,
   GERIUM_STATE_NOT_IMPLEMENTED       = 3,
   GERIUM_STATE_FEATURE_NOT_SUPPORTED = 4,
   GERIUM_STATE_INVALID_ARGUMENT      = 5,
   GERIUM_STATE_MAX_ENUM              = 0x7FFFFFFF
} gerium_state_t;

gerium_public gerium_uint32_t
gerium_version(void);

gerium_public gerium_utf8_t
gerium_version_string(void);

gerium_public gerium_utf8_t
gerium_state_to_string(gerium_state_t state);

GERIUM_END

#endif
