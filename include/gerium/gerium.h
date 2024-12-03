/**
 * \file      gerium.h
 * \brief     gerium API
 * \author    Vladimir Shaleev
 * \copyright MIT License
 */

#ifndef GERIUM_H
#define GERIUM_H

#include "gerium-core.h"

#ifdef GERIUM_PLATFORM_ANDROID
# include "gerium-android.h"
#endif

#ifdef GERIUM_PLATFORM_IOS
# include "gerium-ios.h"
#endif

#ifdef GERIUM_PLATFORM_WEB
# include "gerium-web.h"
#endif

#ifdef GERIUM_PLATFORM_WINDOWS
# include "gerium-windows.h"
#endif

#ifdef GERIUM_PLATFORM_LINUX
# include "gerium-linux.h"
#endif

#endif
