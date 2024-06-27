/**
 * \file      gerium-windows.h
 * \brief     gerium API Core
 * \author    Vladimir Shaleev
 * \copyright MIT License
 */

#ifndef GERIUM_WINDOWS_H
#define GERIUM_WINDOWS_H

#include "gerium-core.h"

#include <Windows.h>

GERIUM_BEGIN

gerium_public gerium_result_t
gerium_windows_application_create(gerium_utf8_t title,
                                  gerium_uint32_t width,
                                  gerium_uint32_t height,
                                  // gerium_application_mode_flags_t mode,
                                  HINSTANCE instance,
                                  gerium_application_t* application);

GERIUM_END

#endif
