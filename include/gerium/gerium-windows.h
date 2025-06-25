/**
 * @file      gerium-windows.h
 * @brief     gerium API Windows
 * @details   Windows interfaces.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @copyright MIT License
 */
#ifndef GERIUM_WINDOWS_H
#define GERIUM_WINDOWS_H

#ifdef GERIUM_PLATFORM_WINDOWS

#include "gerium-core.h"

#include <Windows.h>

GERIUM_BEGIN

#pragma comment(linker, "/export:main")

/**
 * @brief      Creates new application instance.
 * @details    Initializes window system and core subsystems.
 * @param[in]  title Window title.
 * @param[in]  width Initial window width.
 * @param[in]  height Initial window height.
 * @param[in]  instance Win32 API HINSTANCE.
 * @param[out] application Created application instance.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_windows_application_create(gerium_utf8_t title,
                                  gerium_uint32_t width,
                                  gerium_uint32_t height,
                                  HINSTANCE instance,
                                  gerium_application_t* application);

GERIUM_END

#endif

#endif
