/**
 * @file      gerium-fidelityfx.h
 * @brief     gerium API FidelityFX
 * @details   FidelityFX interface.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @copyright MIT License
 */
#ifndef GERIUM_FIDELITYFX_H
#define GERIUM_FIDELITYFX_H

#include "gerium-core.h"

#include <FidelityFX/host/ffx_brixelizer.h>
#include <FidelityFX/host/ffx_brixelizergi.h>
#include <FidelityFX/host/ffx_error.h>
#include <FidelityFX/host/ffx_interface.h>
#include <FidelityFX/host/ffx_types.h>

GERIUM_BEGIN

/**
 * @brief      Create FFX interface.
 * @details    Create FidelityFX FfxInterface.
 * @param[in]  renderer Gerium renderer instance.
 * @param[in]  max_contexts Each ffx component requires its own context.
 * @param[out] ffx_interface Created FidelityFX interface.
 * @return     Operation result
 */
gerium_api gerium_result_t
gerium_renderer_create_ffx_interface(gerium_renderer_t renderer,
                                     gerium_uint32_t max_contexts,
                                     FfxInterface* ffx_interface);

/**
 * @brief     Destroy FFX interface.
 * @details   Destroy FidelityFX FfxInterface.
 * @param[in] renderer Gerium renderer instance.
 * @param[in] ffx_interface FidelityFX interface.
 */
gerium_api void
gerium_renderer_destroy_ffx_interface(gerium_renderer_t renderer,
                                      FfxInterface* ffx_interface);

/**
 * @brief     Wait all FidelityFX jobs.
 * @details   Needed to wait for all FidelityFX jobs to complete before destroying the application.
 * @param[in] renderer Gerium renderer instance.
 */
gerium_api void
gerium_renderer_wait_ffx_jobs(gerium_renderer_t renderer);

/**
 * @brief     Get ffx resource from Gerium buffer.
 * @details   Get FidelityFX resource from Gerium buffer handle.
 * @param[in] renderer Gerium renderer instance.
 * @param[in] handle Gerium buffer handle.
 * @return    FidelityFX resource
 */
gerium_api FfxResource
gerium_renderer_get_ffx_buffer(gerium_renderer_t renderer,
                               gerium_buffer_h handle);

/**
 * @brief     Get ffx resource from Gerium texture.
 * @details   Get FidelityFX resource from Gerium texture handle.
 * @param[in] renderer Gerium renderer instance.
 * @param[in] handle Gerium texture handle.
 * @return    FidelityFX resource
 */
gerium_api FfxResource
gerium_renderer_get_ffx_texture(gerium_renderer_t renderer,
                                gerium_texture_h handle);

/**
 * @brief     Get ffx command list from Gerium command buffer.
 * @details   Get FidelityFX FfxCommandList from Gerium command buffer.
 * @param[in] command_buffer Gerium command buffer.
 * @return    FidelityFX command list
 */
gerium_api FfxCommandList
gerium_command_buffer_get_ffx_command_list(gerium_command_buffer_t command_buffer);

GERIUM_END

#endif
