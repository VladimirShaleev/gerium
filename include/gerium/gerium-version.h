/**
 * @file      gerium-version.h
 * @brief     Library version information and utilities.
 * @details   This header provides version information for the Gerium library,
 *            including version number components and macros for version comparison
 *            and string generation. It supports:
 *            - Major/Minor/Micro version components
 *            - Integer version encoding
 *            - String version generation
 *            
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @copyright MIT License
 */
#ifndef GERIUM_VERSION_H
#define GERIUM_VERSION_H

/**
 * @name  Version Components
 * @brief Individual components of the library version
 * @{
 */

/**
 * @brief Major version number (API-breaking changes)
 * @sa    GERIUM_VERSION
 * @sa    GERIUM_VERSION_STRING
 */
#define GERIUM_VERSION_MAJOR 1

/**
 * @brief Minor version number (backwards-compatible additions)
 * @sa    GERIUM_VERSION
 * @sa    GERIUM_VERSION_STRING
 */
#define GERIUM_VERSION_MINOR 0

/**
 * @brief Micro version number (bug fixes and patches)
 * @sa    GERIUM_VERSION
 * @sa    GERIUM_VERSION_STRING
 */
#define GERIUM_VERSION_MICRO 0

/** @} */

/**
 * @name  Version Utilities
 * @brief Macros for working with version numbers
 * @{
 */

/**
 * @brief     Encodes version components into a single integer
 * @details   Combines major, minor, and micro versions into a 32-bit value:
 *            - Bits 24-31: Major version
 *            - Bits 16-23: Minor version
 *            - Bits 0-15: Micro version
 * @param[in] major Major version number
 * @param[in] minor Minor version number
 * @param[in] micro Micro version number
 * @return    Encoded version as unsigned long
 * @sa        GERIUM_VERSION
 */
#define GERIUM_VERSION_ENCODE(major, minor, micro) (((unsigned long) major) << 16 | (minor) << 8 | (micro))

/**
 * @brief     Internal macro for string version generation
 * @details   Helper macro that stringizes version components (e.g., 1, 0, 0 -> "1.0.0")
 * @param[in] major Major version number
 * @param[in] minor Minor version number
 * @param[in] micro Micro version number
 * @return    Stringified version
 * @note      For internal use only
 * @private
 */
#define GERIUM_VERSION_STRINGIZE_(major, minor, micro) #major "." #minor "." #micro

/**
 * @def       GERIUM_VERSION_STRINGIZE
 * @brief     Creates version string from components
 * @details   Generates a string literal from version components (e.g., 1, 0, 0 -> "1.0.0")
 * @param[in] major Major version number
 * @param[in] minor Minor version number
 * @param[in] micro Micro version number
 * @return    Stringified version
 * @sa        GERIUM_VERSION_STRING
 */
#define GERIUM_VERSION_STRINGIZE(major, minor, micro)  GERIUM_VERSION_STRINGIZE_(major, minor, micro)

/** @} */

/**
 * @name  Current Version
 * @brief Macros representing the current library version
 * @{
 */

/**
 * @brief   Encoded library version as integer
 * @details Combined version value suitable for numeric comparisons.
 *          Use #GERIUM_VERSION_STRING for human-readable format.
 * @sa      GERIUM_VERSION_STRING
 */
#define GERIUM_VERSION GERIUM_VERSION_ENCODE( \
    GERIUM_VERSION_MAJOR, \
    GERIUM_VERSION_MINOR, \
    GERIUM_VERSION_MICRO)

/**
 * @def     GERIUM_VERSION_STRING
 * @brief   Library version as human-readable string
 * @details Version string in "MAJOR.MINOR.MICRO" format (e.g., "1.0.0").
 *          Use #GERIUM_VERSION for numeric comparisons.
 * @sa      GERIUM_VERSION
 */
#define GERIUM_VERSION_STRING GERIUM_VERSION_STRINGIZE( \
    GERIUM_VERSION_MAJOR, \
    GERIUM_VERSION_MINOR, \
    GERIUM_VERSION_MICRO)

/** @} */

#endif /* GERIUM_VERSION_H */
