/**
 * \file      gerium-android.h
 * \brief     gerium API Core
 * \author    Vladimir Shaleev
 * \copyright MIT License
 */

#ifndef GERIUM_ANDROID_H
#define GERIUM_ANDROID_H

#include "gerium-core.h"

#include <android/native_window.h>
#include <android/configuration.h>

#if GERIUM_ANDROID_HAS_NATIVE_APP_GLUE
# include <android_native_app_glue.h>
#endif

GERIUM_BEGIN

GERIUM_END

#endif
