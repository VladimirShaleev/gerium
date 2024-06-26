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

#endif
