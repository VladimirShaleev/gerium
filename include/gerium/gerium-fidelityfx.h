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

GERIUM_BEGIN

gerium_public FfxBrixelizerContext*
gerium_renderer_get_ffx_brixelizer_context(gerium_renderer_t renderer);

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
