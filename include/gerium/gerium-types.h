/**
 * @file      gerium-types.h
 * @brief     Core type definitions for the Gerium framework.
 * @details   This header defines the fundamental object types and handles used throughout
 *            the Gerium framework. It provides forward declarations for all major system
 *            components using opaque pointer types (#GERIUM_TYPE) and index-based handles
 *            (#GERIUM_HANDLE) for better type safety and abstraction.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @copyright MIT License
 */
#ifndef GERIUM_TYPES_H
#define GERIUM_TYPES_H

#include "gerium-platform.h"

GERIUM_BEGIN

/**
 * @name    Opaque Object Types
 * @brief   Forward declarations for framework objects using opaque pointer types
 * @details These macros generate typedefs for pointers to incomplete struct types,
 *          providing type safety while hiding implementation details. Each represents
 *          a major subsystem in the Gerium framework.
 * @sa      GERIUM_TYPE
 * @{
 */
GERIUM_TYPE(gerium_logger)         /**< Logging system interface. */
GERIUM_TYPE(gerium_file)           /**< File I/O abstraction. */
GERIUM_TYPE(gerium_mutex)          /**< Thread synchronization primitive. */
GERIUM_TYPE(gerium_signal)         /**< Event/signal handling. */
GERIUM_TYPE(gerium_application)    /**< Main application controller. */
GERIUM_TYPE(gerium_renderer)       /**< Graphics rendering subsystem. */
GERIUM_TYPE(gerium_command_buffer) /**< GPU command recording interface. */
GERIUM_TYPE(gerium_frame_graph)    /**< Render pass dependency graph. */
GERIUM_TYPE(gerium_profiler)       /**< Performance measurement tool. */
/** @} */

/**
 * @name    Resource Handles
 * @brief   Index-based handles
 * @details These macros generate lightweight handle types,
 *          using indices rather than pointers for better memory management
 *          and cross-API compatibility. Each handle contains an internal index.
 * @sa      GERIUM_HANDLE
 * @{
 */
GERIUM_HANDLE(gerium_buffer)         /**< Handle to GPU buffer resource. */
GERIUM_HANDLE(gerium_texture)        /**< Handle to GPU texture resource. */
GERIUM_HANDLE(gerium_technique)      /**< Handle to rendering technique. */
GERIUM_HANDLE(gerium_descriptor_set) /**< Handle to resource binding set. */
/** @} */

GERIUM_END

#endif /* GERIUM_TYPES_H */
