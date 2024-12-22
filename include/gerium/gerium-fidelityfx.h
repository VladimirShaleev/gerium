/**
 * \file      gerium-fidelityfx.h
 * \brief     gerium API FidelityFX
 * \author    Vladimir Shaleev
 * \copyright MIT License
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

gerium_public gerium_result_t
gerium_renderer_create_ffx_interface(gerium_renderer_t renderer,
                                     gerium_uint32_t max_contexts,
                                     FfxInterface* ffx_interface);

gerium_public void
gerium_renderer_destroy_ffx_interface(gerium_renderer_t renderer,
                                      FfxInterface* ffx_interface);

gerium_public void
gerium_renderer_wait_ffx_jobs(gerium_renderer_t renderer);

gerium_public FfxResource
gerium_renderer_get_ffx_buffer(gerium_renderer_t renderer,
                               gerium_buffer_h handle);

gerium_public FfxResource
gerium_renderer_get_ffx_texture(gerium_renderer_t renderer,
                                gerium_texture_h handle);

gerium_public FfxCommandList
gerium_command_buffer_get_ffx_command_list(gerium_command_buffer_t command_buffer);

GERIUM_END

#endif
