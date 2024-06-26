/**
 * \file      gerium-version.h
 * \brief     define \a version of library
 * \author    Vladimir Shaleev
 * \copyright MIT License
 */

#ifndef GERIUM_VERSION_H
#define GERIUM_VERSION_H

#define GERIUM_VERSION_MAJOR 0
#define GERIUM_VERSION_MINOR 0
#define GERIUM_VERSION_MICRO 0

#define GERIUM_VERSION_ENCODE(major, minor, micro) (((unsigned long) major) << 16 | (minor) << 8 | (micro))

#define GERIUM_VERSION_STRINGIZE_(major, minor, micro) #major "." #minor "." #micro
#define GERIUM_VERSION_STRINGIZE(major, minor, micro)  GERIUM_VERSION_STRINGIZE_(major, minor, micro)

#define GERIUM_VERSION GERIUM_VERSION_ENCODE( \
    GERIUM_VERSION_MAJOR, \
    GERIUM_VERSION_MINOR, \
    GERIUM_VERSION_MICRO)

#define GERIUM_VERSION_STRING GERIUM_VERSION_STRINGIZE( \
    GERIUM_VERSION_MAJOR, \
    GERIUM_VERSION_MINOR, \
    GERIUM_VERSION_MICRO)

#endif
