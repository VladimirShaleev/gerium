#include "Application.hpp"

void IndirectPass::render(gerium_frame_graph_t frameGraph,
                          gerium_renderer_t renderer,
                          gerium_command_buffer_t commandBuffer,
                          gerium_uint32_t worker,
                          gerium_uint32_t totalWorkers) {
    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet, SCENE_DATA_SET);
    gerium_command_buffer_dispatch(commandBuffer, 1, 1, 1);
}

void IndirectPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet          = application()->resourceManager().createDescriptorSet("", true);
    const auto commandCount = !_latePass ? "command_count" : "command_count_late";
    gerium_renderer_bind_resource(renderer, _descriptorSet, 0, commandCount, false);
    gerium_renderer_bind_resource(renderer, _descriptorSet, 1, "commands", false);
}

void IndirectPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
}

void DepthPyramidPass::render(gerium_frame_graph_t frameGraph,
                              gerium_renderer_t renderer,
                              gerium_command_buffer_t commandBuffer,
                              gerium_uint32_t worker,
                              gerium_uint32_t totalWorkers) {
    gerium_texture_h depth;
    gerium_renderer_get_texture(renderer, "depth", false, &depth);
    assignReductionSampler(renderer, depth);

    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());

    for (gerium_uint16_t m = 0; m < _depthPyramidMipLevels; ++m) {
        auto levelWidth  = std::max(1, _depthPyramidWidth >> m);
        auto levelHeight = std::max(1, _depthPyramidHeight >> m);

        gerium_command_buffer_barrier_texture_write(commandBuffer, _depthPyramidMips[m]);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSets[m], 0);
        gerium_command_buffer_dispatch(commandBuffer, getGroupCount(levelWidth, 32), getGroupCount(levelHeight, 32), 1);
        gerium_command_buffer_barrier_texture_read(commandBuffer, _depthPyramidMips[m]);
    }
}

void DepthPyramidPass::resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    createDepthPyramid(frameGraph, renderer);
}

void DepthPyramidPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    createDepthPyramid(frameGraph, renderer);
}

void DepthPyramidPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _depthPyramid = nullptr;
    for (auto& mip : _depthPyramidMips) {
        mip = nullptr;
    }
    for (auto& buffer : _imageSizes) {
        buffer = nullptr;
    }
    for (auto& set : _descriptorSets) {
        set = nullptr;
    }
}

void DepthPyramidPass::createDepthPyramid(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    uninitialize(frameGraph, renderer);

    _depthPyramidWidth     = previousPow2(application()->width());
    _depthPyramidHeight    = previousPow2(application()->height());
    _depthPyramidMipLevels = calcMipLevels(_depthPyramidWidth, _depthPyramidHeight);

    gerium_texture_info_t info{};
    info.width   = _depthPyramidWidth;
    info.height  = _depthPyramidHeight;
    info.depth   = 1;
    info.mipmaps = _depthPyramidMipLevels;
    info.format  = GERIUM_FORMAT_R32_SFLOAT;
    info.type    = GERIUM_TEXTURE_TYPE_2D;
    info.name    = "depth_pyramid";

    application()->resourceManager().update(0);
    _depthPyramid = application()->resourceManager().createTexture(info, nullptr, 0);
    assignReductionSampler(renderer, _depthPyramid);

    gerium_frame_graph_add_texture(frameGraph, "depth_pyramid", _depthPyramid);

    for (gerium_uint16_t m = 0; m < _depthPyramidMipLevels; ++m) {
        auto levelWidth  = std::max(1, _depthPyramidWidth >> m);
        auto levelHeight = std::max(1, _depthPyramidHeight >> m);
        auto imageSize   = glm::vec2(levelWidth, levelHeight);
        _imageSizes[m]   = application()->resourceManager().createBuffer(
            GERIUM_BUFFER_USAGE_UNIFORM_BIT, false, "", &imageSize.x, sizeof(imageSize), 0);

        auto name = "depth_pyramid_mip_" + std::to_string(m);

        _depthPyramidMips[m] =
            application()->resourceManager().createTextureView(name, _depthPyramid, GERIUM_TEXTURE_TYPE_2D, m, 1, 0);
        _descriptorSets[m] = application()->resourceManager().createDescriptorSet("", true, 0);

        if (m == 0) {
            gerium_renderer_bind_resource(renderer, _descriptorSets[m], 0, "depth", false);
        } else {
            gerium_renderer_bind_texture(renderer, _descriptorSets[m], 0, 0, _depthPyramidMips[m - 1]);
        }
        gerium_renderer_bind_texture(renderer, _descriptorSets[m], 1, 0, _depthPyramidMips[m]);
        gerium_renderer_bind_buffer(renderer, _descriptorSets[m], 2, _imageSizes[m]);
    }
}

void DepthPyramidPass::assignReductionSampler(gerium_renderer_t renderer, gerium_texture_h texture) {
    gerium_renderer_texture_sampler(renderer,
                                    texture,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_FILTER_NEAREST,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_REDUCTION_MODE_MIN);
}

void CullingPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    auto groupSize = getGroupCount(application()->cluster().instanceCount, 64U);
    auto camera    = application()->getCamera();
    gerium_buffer_h commandCount;
    check(gerium_renderer_get_buffer(renderer, !_latePass ? "command_count" : "command_count_late", &commandCount));
    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet0, GLOBAL_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, application()->cluster().descriptorSet, CLUSTER_DATA_SET);
    gerium_command_buffer_fill_buffer(commandBuffer, commandCount, 0, 4, 0);
    gerium_command_buffer_dispatch(commandBuffer, groupSize, 1, 1);
}

void CullingPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    auto& cluster           = application()->cluster();
    _descriptorSet0         = application()->resourceManager().createDescriptorSet("", true);
    const auto commandCount = !_latePass ? "command_count" : "command_count_late";
    gerium_renderer_bind_buffer(renderer, _descriptorSet0, 0, application()->drawData());
    gerium_renderer_bind_resource(renderer, _descriptorSet0, 1, commandCount, false);
    gerium_renderer_bind_resource(renderer, _descriptorSet0, 2, "commands", false);
    gerium_renderer_bind_resource(renderer, _descriptorSet0, 3, "visibility", false);
    if (_latePass) {
        gerium_renderer_bind_resource(renderer, _descriptorSet0, 4, "depth_pyramid", false);
    }
}

void CullingPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet0 = nullptr;
}

void GBufferPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    auto camera = application()->getCamera();
    gerium_buffer_h commandCount;
    check(gerium_renderer_get_buffer(renderer, !_latePass ? "command_count" : "command_count_late", &commandCount));
    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet, GLOBAL_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, application()->cluster().descriptorSet, CLUSTER_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, application()->texturesSet(), TEXTURE_SET);
    gerium_command_buffer_draw_mesh_tasks_indirect(commandBuffer, commandCount, 4, 1, 12);
}

void GBufferPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = application()->resourceManager().createDescriptorSet("", true);
    gerium_renderer_bind_resource(renderer, _descriptorSet, 0, "commands", false);
    gerium_renderer_bind_resource(renderer, _descriptorSet, 1, "meshlet_visibility", false);
    if (_latePass) {
        gerium_renderer_bind_resource(renderer, _descriptorSet, 2, "depth_pyramid", false);
    }
}

void GBufferPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
}

void LightPass::render(gerium_frame_graph_t frameGraph,
                       gerium_renderer_t renderer,
                       gerium_command_buffer_t commandBuffer,
                       gerium_uint32_t worker,
                       gerium_uint32_t totalWorkers) {
    static bool drawProfiler = false;

    auto camera  = application()->settings().DebugCamera ? application()->getDebugCamera() : application()->getCamera();
    auto skyDome = application()->getPass<SkyDomeGenPass>();

    auto lightCountBuffer = application()->resourceManager().createBuffer(
        GERIUM_BUFFER_USAGE_UNIFORM_BIT, true, "light_count", nullptr, sizeof(glm::uint), 0);
    auto lightBuffer = application()->resourceManager().createBuffer(
        GERIUM_BUFFER_USAGE_STORAGE_BIT, true, "lights", nullptr, sizeof(Light) * 1, 0);

    auto lightCount = (glm::uint*) gerium_renderer_map_buffer(renderer, lightCountBuffer, 0, sizeof(glm::uint));
    *lightCount = 1;
    gerium_renderer_unmap_buffer(renderer, lightCountBuffer);

    auto lights = (Light*) gerium_renderer_map_buffer(renderer, lightBuffer, 0, sizeof(Light) * 1);
    lights[0].directionRange = skyDome->skyData().sunDirection;
    lights[0].colorIntensity = vec4(1.0, 1.0, 1.0, 5.0);
    lights[0].type           = LIGHT_TYPE_DIRECTIONAL;
    gerium_renderer_unmap_buffer(renderer, lightBuffer);

    auto ds                 = application()->resourceManager().createDescriptorSet("");
    auto& settings          = application()->settings();
    const auto albedoTexure = application()->settings().DebugCamera ? "debug_albedo" : "albedo";
    const auto normalTexure = application()->settings().DebugCamera ? "debug_normal" : "normal";
    const auto aoRoughnessMetallicTexure =
        application()->settings().DebugCamera ? "debug_ao_roughness_metallic" : "ao_roughness_metallic";
    const auto motionTexure = application()->settings().DebugCamera ? "debug_depth" : "depth";
    gerium_renderer_bind_texture(renderer, ds, 0, 0, application()->brdfLut());
    gerium_renderer_bind_buffer(renderer, ds, 1, lightCountBuffer);
    gerium_renderer_bind_buffer(renderer, ds, 2, lightBuffer);
    gerium_renderer_bind_resource(renderer, ds, 3, albedoTexure, false);
    gerium_renderer_bind_resource(renderer, ds, 4, normalTexure, false);
    gerium_renderer_bind_resource(renderer, ds, 5, aoRoughnessMetallicTexure, false);
    gerium_renderer_bind_resource(renderer, ds, 6, motionTexure, false);
    gerium_renderer_bind_resource(renderer, ds, 7, "diffuse_gi", false);
    gerium_renderer_bind_resource(renderer, ds, 8, "specular_gi", false);

    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, ds, GLOBAL_DATA_SET);
    gerium_command_buffer_draw(commandBuffer, 0, 3, 0, 1);
}

void PresentPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    static bool drawProfiler = false;

    auto& settings = application()->settings();
    auto ds        = application()->resourceManager().createDescriptorSet("");
    gerium_renderer_bind_resource(renderer, ds, 0, "color", false);

    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, ds, 0);
    gerium_command_buffer_draw(commandBuffer, 0, 3, 0, 1);

    if (drawProfiler) {
        gerium_bool_t show = drawProfiler;
        gerium_command_buffer_draw_profiler(commandBuffer, &show);
        drawProfiler = show;
    }

    if (ImGui::Begin("Settings")) {
        ImGui::Text("Meshlets: %s", application()->meshShaderSupported() ? "hardware" : "software");
        ImGui::Separator();
        static int hour = 7;
        ImGui::SliderInt("Hour", &settings.Hour, 5, 20, "%d", ImGuiSliderFlags_AlwaysClamp);
    }

    ImGui::End();
}

void DebugOcclusionPass::render(gerium_frame_graph_t frameGraph,
                                gerium_renderer_t renderer,
                                gerium_command_buffer_t commandBuffer,
                                gerium_uint32_t worker,
                                gerium_uint32_t totalWorkers) {
    // On the debug pass (debug camera) we simply call vkCmdDrawMeshTasksIndirectEXT
    // again with the indirect draw buffer already filled. This works because we already
    // have the visibility buffers filled visibility and meshletVisibility after LATE passes
    // of culling.comp.glsl and gbuffer.task.glsl shaders. We just change the view and
    // projection matrices to the debug camera
    auto camera = application()->getDebugCamera();
    gerium_buffer_h commandCount;
    check(gerium_renderer_get_buffer(renderer, "command_count_late", &commandCount));
    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet, GLOBAL_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, application()->cluster().descriptorSet, CLUSTER_DATA_SET);
    gerium_command_buffer_draw_mesh_tasks_indirect(commandBuffer, commandCount, 4, 1, 12);
}

void DebugOcclusionPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    auto& cluster  = application()->cluster();
    _descriptorSet = application()->resourceManager().createDescriptorSet("", true);
    gerium_renderer_bind_resource(renderer, _descriptorSet, 0, "commands", false);
    gerium_renderer_bind_resource(renderer, _descriptorSet, 1, "meshlet_visibility", false);
}

void DebugOcclusionPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
}

void DebugLinePass::render(gerium_frame_graph_t frameGraph,
                           gerium_renderer_t renderer,
                           gerium_command_buffer_t commandBuffer,
                           gerium_uint32_t worker,
                           gerium_uint32_t totalWorkers) {
    if (application()->settings().DebugCamera) {
        gerium_uint32_t vertices = 0;

        auto dataV = (glm::vec3*) gerium_renderer_map_buffer(renderer, _vertices, 0, sizeof(glm::vec3) * _maxPoints);

        auto primaryCamera = application()->getCamera();

        const auto nearPlane = primaryCamera->nearPlane();
        const auto farPlane  = 0.999f;

        const auto aspect = float(application()->width()) / application()->height();
        const auto persp =
            glm::perspective(primaryCamera->fov(), aspect, primaryCamera->nearPlane(), primaryCamera->farPlane());

        auto invV = glm::inverse(persp * primaryCamera->view());

        const glm::vec4 point0 = invV * glm::vec4(-1.0f, -1.0f, nearPlane, 1.0f);
        const glm::vec4 point1 = invV * glm::vec4(-1.0f, -1.0f, farPlane, 1.0f);
        const glm::vec4 point2 = invV * glm::vec4(1.0f, -1.0f, nearPlane, 1.0f);
        const glm::vec4 point3 = invV * glm::vec4(1.0f, -1.0f, farPlane, 1.0f);
        const glm::vec4 point4 = invV * glm::vec4(-1.0f, 1.0f, nearPlane, 1.0f);
        const glm::vec4 point5 = invV * glm::vec4(-1.0f, 1.0f, farPlane, 1.0f);
        const glm::vec4 point6 = invV * glm::vec4(1.0f, 1.0f, nearPlane, 1.0f);
        const glm::vec4 point7 = invV * glm::vec4(1.0f, 1.0f, farPlane, 1.0f);

        dataV[vertices++] = glm::vec3(point0.xyz()) / point0.w;
        dataV[vertices++] = glm::vec3(point1.xyz()) / point1.w;
        dataV[vertices++] = glm::vec3(point2.xyz()) / point2.w;
        dataV[vertices++] = glm::vec3(point3.xyz()) / point3.w;
        dataV[vertices++] = glm::vec3(point4.xyz()) / point4.w;
        dataV[vertices++] = glm::vec3(point5.xyz()) / point5.w;
        dataV[vertices++] = glm::vec3(point6.xyz()) / point6.w;
        dataV[vertices++] = glm::vec3(point7.xyz()) / point7.w;

        dataV[vertices++] = glm::vec3(point0.xyz()) / point0.w;
        dataV[vertices++] = glm::vec3(point2.xyz()) / point2.w;
        dataV[vertices++] = glm::vec3(point2.xyz()) / point2.w;
        dataV[vertices++] = glm::vec3(point6.xyz()) / point6.w;
        dataV[vertices++] = glm::vec3(point6.xyz()) / point6.w;
        dataV[vertices++] = glm::vec3(point4.xyz()) / point4.w;
        dataV[vertices++] = glm::vec3(point4.xyz()) / point4.w;
        dataV[vertices++] = glm::vec3(point0.xyz()) / point0.w;

        dataV[vertices++] = glm::vec3(point1.xyz()) / point1.w;
        dataV[vertices++] = glm::vec3(point3.xyz()) / point3.w;
        dataV[vertices++] = glm::vec3(point3.xyz()) / point3.w;
        dataV[vertices++] = glm::vec3(point7.xyz()) / point7.w;
        dataV[vertices++] = glm::vec3(point7.xyz()) / point7.w;
        dataV[vertices++] = glm::vec3(point5.xyz()) / point5.w;
        dataV[vertices++] = glm::vec3(point5.xyz()) / point5.w;
        dataV[vertices++] = glm::vec3(point1.xyz()) / point1.w;

        gerium_renderer_unmap_buffer(renderer, _vertices);

        auto camera = application()->getDebugCamera();
        gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
        gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), 0);
        gerium_command_buffer_bind_vertex_buffer(commandBuffer, _vertices, 0, 0);
        gerium_command_buffer_draw(commandBuffer, 0, vertices, 0, 1);
    }
}

void DebugLinePass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = application()->resourceManager().createDescriptorSet("");
    gerium_renderer_bind_resource(renderer, _descriptorSet, 0, "debug_meshlet", false);

    _maxPoints = 48;
    _vertices  = application()->resourceManager().createBuffer(
        GERIUM_BUFFER_USAGE_VERTEX_BIT, true, "lines_vertices", nullptr, sizeof(glm::vec3) * _maxPoints);
}

void DebugLinePass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
    _vertices      = nullptr;
}

void BSDFPass::render(gerium_frame_graph_t frameGraph,
                      gerium_renderer_t renderer,
                      gerium_command_buffer_t commandBuffer,
                      gerium_uint32_t worker,
                      gerium_uint32_t totalWorkers) {
    auto camera = application()->settings().DebugCamera ? application()->getDebugCamera() : application()->getCamera();
    auto commandList = gerium_command_buffer_get_ffx_command_list(commandBuffer);

    gerium_buffer_h brickAabbs;
    gerium_buffer_h scratchBuffer;
    check(gerium_renderer_get_buffer(renderer, "brick_aabbs", &brickAabbs));
    check(gerium_renderer_get_buffer(renderer, "ffx_scratch_buffer", &scratchBuffer));

    size_t scratchBufferSize = 0;

    FfxBrixelizerStats stats{};

    FfxBrixelizerUpdateDescription desc{};
    desc.frameIndex           = _frameIndex++;
    desc.sdfCenter[0]         = camera->position().x;
    desc.sdfCenter[1]         = camera->position().y;
    desc.sdfCenter[2]         = camera->position().z;
    desc.maxReferences        = 32 * (1 << 20);
    desc.maxBricksPerBake     = 1 << 14;
    desc.triangleSwapSize     = 300 * (1 << 20);
    desc.outScratchBufferSize = &scratchBufferSize;
    desc.outStats             = &stats;

    desc.resources.sdfAtlas   = gerium_renderer_get_ffx_texture(renderer, bsdfAtlas());
    desc.resources.brickAABBs = gerium_renderer_get_ffx_buffer(renderer, brickAabbs);
    for (uint32_t i = 0; i < kFfxBrixelizerMaxCascades; ++i) {
        gerium_command_buffer_barrier_buffer_write(commandBuffer, cascadeAABBTrees(i));
        gerium_command_buffer_barrier_buffer_write(commandBuffer, cascadeBrickMaps(i));
        desc.resources.cascadeResources[i].aabbTree = gerium_renderer_get_ffx_buffer(renderer, cascadeAABBTrees(i));
        desc.resources.cascadeResources[i].brickMap = gerium_renderer_get_ffx_buffer(renderer, cascadeBrickMaps(i));
    }

    auto cascadeIndexOffset = 2 * kNumBrixelizerCascades;

    // gerium_texture_h debugBSDF;
    // check(gerium_renderer_get_texture(renderer, "debug_bsdf", false, &debugBSDF));
    // FfxBrixelizerDebugVisualizationDescription debugDesc{};
    // const auto inverseView       = glm::inverse(camera->view());
    // const auto inverseProjection = glm::inverse(camera->projection());
    // memcpy(&debugDesc.inverseViewMatrix, &inverseView[0][0], sizeof(debugDesc.inverseViewMatrix));
    // memcpy(&debugDesc.inverseProjectionMatrix, &inverseProjection[0][0], sizeof(debugDesc.inverseProjectionMatrix));
    // debugDesc.debugState        = FFX_BRIXELIZER_TRACE_DEBUG_MODE_ITERATIONS;
    // debugDesc.startCascadeIndex = cascadeIndexOffset;
    // debugDesc.endCascadeIndex   = cascadeIndexOffset + kNumBrixelizerCascades - 1;
    // debugDesc.tMin              = 0.0f;
    // debugDesc.tMax              = 100000.0f;
    // debugDesc.sdfSolveEps       = 0.5f;
    // debugDesc.renderWidth       = application()->width();
    // debugDesc.renderHeight      = application()->height();
    // debugDesc.output            = gerium_renderer_get_ffx_texture(renderer, debugBSDF);
    //
    // FfxBrixelizerPopulateDebugAABBsFlags populateDebugAABBFlags = FFX_BRIXELIZER_POPULATE_AABBS_NONE;

    // if (1) {
    //     populateDebugAABBFlags =
    //         (FfxBrixelizerPopulateDebugAABBsFlags) (populateDebugAABBFlags |
    //                                                 FFX_BRIXELIZER_POPULATE_AABBS_STATIC_INSTANCES);
    // }

    // if (1) {
    //     populateDebugAABBFlags = (FfxBrixelizerPopulateDebugAABBsFlags) (populateDebugAABBFlags |
    //                                                                      FFX_BRIXELIZER_POPULATE_AABBS_CASCADE_AABBS);
    // }

    // desc.debugVisualizationDesc  = &debugDesc;
    // desc.populateDebugAABBsFlags = populateDebugAABBFlags;

    auto scratch = gerium_renderer_get_ffx_buffer(renderer, scratchBuffer);
    // scratch.description.stride = 16;

    ffxBrixelizerBakeUpdate(brixelizerContext(), &desc, &_brixelizerBakedUpdateDesc);
    assert(scratchBufferSize < (1 << 30) &&
           L"Required Brixelizer scratch memory size larger than available GPU buffer.");

    ffxBrixelizerUpdate(brixelizerContext(), &_brixelizerBakedUpdateDesc, scratch, commandList);
}

void BSDFPass::registerResources(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    gerium_texture_info_t info{};
    info.width   = FFX_BRIXELIZER_STATIC_CONFIG_SDF_ATLAS_SIZE;
    info.height  = FFX_BRIXELIZER_STATIC_CONFIG_SDF_ATLAS_SIZE;
    info.depth   = FFX_BRIXELIZER_STATIC_CONFIG_SDF_ATLAS_SIZE;
    info.mipmaps = 1;
    info.format  = GERIUM_FORMAT_R8_UNORM;
    info.type    = GERIUM_TEXTURE_TYPE_3D;
    info.name    = "bsdf_atlas";
    _bsdfAtlas   = application()->resourceManager().createTexture(info, nullptr);

    gerium_renderer_texture_sampler(renderer,
                                    _bsdfAtlas,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_FILTER_NEAREST,
                                    GERIUM_ADDRESS_MODE_REPEAT,
                                    GERIUM_ADDRESS_MODE_REPEAT,
                                    GERIUM_ADDRESS_MODE_REPEAT,
                                    GERIUM_REDUCTION_MODE_WEIGHTED_AVERAGE);

    gerium_frame_graph_add_texture(frameGraph, info.name, _bsdfAtlas);

    for (uint32_t i = 0; i < kFfxBrixelizerMaxCascades; ++i) {
        auto index = std::to_string(i);

        _cascadeAABBTrees[i] = application()->resourceManager().createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                                             false,
                                                                             "cascade_aabb_tree_" + index,
                                                                             nullptr,
                                                                             FFX_BRIXELIZER_CASCADE_AABB_TREE_SIZE);
        _cascadeBrickMaps[i] = application()->resourceManager().createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                                             false,
                                                                             "cascade_brick_map_" + index,
                                                                             nullptr,
                                                                             FFX_BRIXELIZER_CASCADE_BRICK_MAP_SIZE);
    }

    check(gerium_renderer_create_ffx_interface(renderer, 2, &_brixelizerParams.backendInterface));

    _brixelizerParams.sdfCenter[0] = 0.0f;
    _brixelizerParams.sdfCenter[1] = 0.0f;
    _brixelizerParams.sdfCenter[2] = 0.0f;
    _brixelizerParams.numCascades  = kNumBrixelizerCascades;
    // _brixelizerParams.flags        = FFX_BRIXELIZER_CONTEXT_FLAG_ALL_DEBUG;

    auto voxelSize = 1.0f;
    for (uint32_t i = 0; i < _brixelizerParams.numCascades; ++i) {
        auto& cascade     = _brixelizerParams.cascadeDescs[i];
        cascade.flags     = (FfxBrixelizerCascadeFlag) (FFX_BRIXELIZER_CASCADE_STATIC | FFX_BRIXELIZER_CASCADE_DYNAMIC);
        cascade.voxelSize = voxelSize;
        voxelSize *= 2.0f;
    }

    ffxCheck(ffxBrixelizerContextCreate(&_brixelizerParams, &_brixelizerContext));
}

void BSDFPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    if (_brixelizerInstanceIds.size()) {
        ffxBrixelizerDeleteInstances(
            brixelizerContext(), _brixelizerInstanceIds.data(), (uint32_t) _brixelizerInstanceIds.size());
        _brixelizerInstanceIds.clear();
        _brixelizerInstances.clear();
    }

    if (_brixelizerBufferIds.size()) {
        ffxBrixelizerUnregisterBuffers(
            brixelizerContext(), _brixelizerBufferIds.data(), (uint32_t) _brixelizerBufferIds.size());
        _brixelizerBufferIds.clear();
        _brixelizerBuffers.clear();
    }
    ffxBrixelizerContextDestroy(&_brixelizerContext);
    gerium_renderer_destroy_ffx_interface(renderer, &_brixelizerParams.backendInterface);

    _cascadeBrickMaps = {};
    _cascadeAABBTrees = {};
    _bsdfAtlas        = {};
}

uint32_t BSDFPass::registerBuffer(gerium_renderer_t renderer, const Buffer& handle) {
    _brixelizerBuffers.push_back(handle);
    _brixelizerBufferIds.push_back({});

    FfxBrixelizerBufferDescription desc{};
    desc.buffer   = gerium_renderer_get_ffx_buffer(renderer, handle);
    desc.outIndex = &_brixelizerBufferIds.back();
    ffxCheck(ffxBrixelizerRegisterBuffers(&_brixelizerContext, &desc, 1));
    return _brixelizerBufferIds.back();
}

FfxBrixelizerInstanceID BSDFPass::createInstance(const FfxBrixelizerInstanceDescription& instance) {
    _brixelizerInstances.push_back(instance);
    auto& desc = _brixelizerInstances.back();

    _brixelizerInstanceIds.push_back({});
    desc.outInstanceID = &_brixelizerInstanceIds.back();

    ffxCheck(ffxBrixelizerCreateInstances(brixelizerContext(), &desc, 1));
    return *desc.outInstanceID;
}

void BGIPass::render(gerium_frame_graph_t frameGraph,
                     gerium_renderer_t renderer,
                     gerium_command_buffer_t commandBuffer,
                     gerium_uint32_t worker,
                     gerium_uint32_t totalWorkers) {
    auto bsdfPass = application()->getPass<BSDFPass>();
    auto camera = application()->settings().DebugCamera ? application()->getDebugCamera() : application()->getCamera();
    auto commandList = gerium_command_buffer_get_ffx_command_list(commandBuffer);

    gerium_texture_h depthTex;
    gerium_texture_h prevDepthTex;
    gerium_texture_h normalTex;
    gerium_texture_h prevNormalTex;
    gerium_texture_h roughnessTex;
    gerium_texture_h motionTex;
    gerium_texture_h prevLitTex;
    gerium_texture_h skydomePrefilteredEnvTex;
    gerium_texture_h diffuseGITex;
    gerium_texture_h specularGITex;
    gerium_texture_h noiseTex = application()->noiseTexture(_frameIndex++);
    gerium_texture_h debugGITex;
    gerium_buffer_h brickAabbs;

    check(gerium_renderer_get_texture(renderer, "depth", false, &depthTex));
    check(gerium_renderer_get_texture(renderer, "depth", true, &prevDepthTex));
    check(gerium_renderer_get_texture(renderer, "normal", false, &normalTex));
    check(gerium_renderer_get_texture(renderer, "normal", true, &prevNormalTex));
    check(gerium_renderer_get_texture(renderer, "ao_roughness_metallic", false, &roughnessTex));
    check(gerium_renderer_get_texture(renderer, "motion", false, &motionTex));
    check(gerium_renderer_get_texture(renderer, "light", true, &prevLitTex));
    check(gerium_renderer_get_texture(renderer, "skydome_prefiltered_env", false, &skydomePrefilteredEnvTex));
    check(gerium_renderer_get_texture(renderer, "diffuse_gi", false, &diffuseGITex));
    check(gerium_renderer_get_texture(renderer, "specular_gi", false, &specularGITex));
    check(gerium_renderer_get_texture(renderer, "debug_gi", false, &debugGITex));
    check(gerium_renderer_get_buffer(renderer, "brick_aabbs", &brickAabbs));

    memcpy(&_dispatchDesc.view, &camera->view()[0][0], sizeof(_dispatchDesc.view));
    memcpy(&_dispatchDesc.projection, &camera->projection()[0][0], sizeof(_dispatchDesc.projection));
    memcpy(&_dispatchDesc.prevView, &camera->prevView()[0][0], sizeof(_dispatchDesc.prevView));
    memcpy(&_dispatchDesc.prevProjection, &camera->prevProjection()[0][0], sizeof(_dispatchDesc.prevProjection));
    memcpy(&_dispatchDesc.cameraPosition, &camera->position()[0], sizeof(_dispatchDesc.cameraPosition));

    _dispatchDesc.startCascade        = 2 * kNumBrixelizerCascades;
    _dispatchDesc.endCascade          = _dispatchDesc.startCascade + kNumBrixelizerCascades - 1;
    _dispatchDesc.rayPushoff          = 0.25f;
    _dispatchDesc.sdfSolveEps         = 0.5f;
    _dispatchDesc.specularRayPushoff  = 0.25f;
    _dispatchDesc.specularSDFSolveEps = 0.5f;
    _dispatchDesc.tMin                = 0.0f;
    _dispatchDesc.tMax                = 100000.0f;

    _dispatchDesc.environmentMap = gerium_renderer_get_ffx_texture(renderer, skydomePrefilteredEnvTex);
    _dispatchDesc.prevLitOutput  = gerium_renderer_get_ffx_texture(renderer, prevLitTex);
    _dispatchDesc.depth          = gerium_renderer_get_ffx_texture(renderer, depthTex);
    _dispatchDesc.historyDepth   = gerium_renderer_get_ffx_texture(renderer, prevDepthTex);
    _dispatchDesc.normal         = gerium_renderer_get_ffx_texture(renderer, normalTex);
    _dispatchDesc.historyNormal  = gerium_renderer_get_ffx_texture(renderer, prevNormalTex);
    _dispatchDesc.roughness      = gerium_renderer_get_ffx_texture(renderer, roughnessTex);
    _dispatchDesc.motionVectors  = gerium_renderer_get_ffx_texture(renderer, motionTex);
    _dispatchDesc.noiseTexture   = gerium_renderer_get_ffx_texture(renderer, noiseTex);

    _dispatchDesc.normalsUnpackMul        = 1.0f;
    _dispatchDesc.normalsUnpackAdd        = 0.0f;
    _dispatchDesc.isRoughnessPerceptual   = false;
    _dispatchDesc.roughnessChannel        = 1;
    _dispatchDesc.roughnessThreshold      = 0.9f;
    _dispatchDesc.environmentMapIntensity = 0.1f;
    _dispatchDesc.motionVectorScale       = { 1.0f, 1.0f };

    _dispatchDesc.sdfAtlas    = gerium_renderer_get_ffx_texture(renderer, bsdfPass->bsdfAtlas());
    _dispatchDesc.bricksAABBs = gerium_renderer_get_ffx_buffer(renderer, brickAabbs);
    for (uint32_t i = 0; i < kFfxBrixelizerMaxCascades; ++i) {
        gerium_command_buffer_barrier_buffer_read(commandBuffer, bsdfPass->cascadeAABBTrees(i));
        gerium_command_buffer_barrier_buffer_read(commandBuffer, bsdfPass->cascadeBrickMaps(i));
        _dispatchDesc.cascadeAABBTrees[i] = gerium_renderer_get_ffx_buffer(renderer, bsdfPass->cascadeAABBTrees(i));
        _dispatchDesc.cascadeBrickMaps[i] = gerium_renderer_get_ffx_buffer(renderer, bsdfPass->cascadeBrickMaps(i));
    }

    _dispatchDesc.outputDiffuseGI  = gerium_renderer_get_ffx_texture(renderer, diffuseGITex);
    _dispatchDesc.outputSpecularGI = gerium_renderer_get_ffx_texture(renderer, specularGITex);

    ffxBrixelizerGetRawContext(bsdfPass->brixelizerContext(), &_dispatchDesc.brixelizerContext);
    ffxBrixelizerGIContextDispatch(&_brixelizerGIContext, &_dispatchDesc, commandList);

    // FfxBrixelizerGIDebugDescription desc{};
    // memcpy(&desc.view, &camera->view()[0][0], sizeof(desc.view));
    // memcpy(&desc.projection, &camera->projection()[0][0], sizeof(desc.projection));

    // auto cascadeIndexOffset = 2 * kNumBrixelizerCascades;
    // desc.startCascade       = cascadeIndexOffset;
    // desc.endCascade         = cascadeIndexOffset + kNumBrixelizerCascades - 1;

    // desc.outputSize[0]    = application()->width();
    // desc.outputSize[1]    = application()->height();
    // desc.debugMode        = FFX_BRIXELIZER_GI_DEBUG_MODE_RADIANCE_CACHE; //
    // FFX_BRIXELIZER_GI_DEBUG_MODE_IRRADIANCE_CACHE; desc.normalsUnpackMul = 1.0f; desc.normalsUnpackAdd = 0.0f;
    // desc.depth            = gerium_renderer_get_ffx_texture(renderer, depthTex);
    // desc.normal           = gerium_renderer_get_ffx_texture(renderer, normalTex);
    // desc.sdfAtlas         = gerium_renderer_get_ffx_texture(renderer, bsdfPass->bsdfAtlas());
    // desc.bricksAABBs      = gerium_renderer_get_ffx_buffer(renderer, brickAabbs);

    // for (uint32_t i = 0; i < kFfxBrixelizerMaxCascades; ++i) {
    //     desc.cascadeAABBTrees[i] = gerium_renderer_get_ffx_buffer(renderer, bsdfPass->cascadeAABBTrees(i));
    //     desc.cascadeBrickMaps[i] = gerium_renderer_get_ffx_buffer(renderer, bsdfPass->cascadeBrickMaps(i));
    // }

    // desc.outputDebug = gerium_renderer_get_ffx_texture(renderer, debugGITex);

    // desc.brixelizerContext = _dispatchDesc.brixelizerContext;

    // ffxCheck(ffxBrixelizerGIContextDebugVisualization(&_brixelizerGIContext, &desc, commandList));
}

void BGIPass::registerResources(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    createContext();
}

void BGIPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    destroyContext(renderer);
}

void BGIPass::resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    destroyContext(renderer);
    createContext();
}

void BGIPass::createContext() {
    auto bsdfPass = application()->getPass<BSDFPass>();

    FfxBrixelizerGIContextDescription desc = {};
    desc.flags                             = FFX_BRIXELIZER_GI_FLAG_DEPTH_INVERTED;
    desc.internalResolution                = FFX_BRIXELIZER_GI_INTERNAL_RESOLUTION_50_PERCENT;
    desc.displaySize                       = { application()->width(), application()->height() };
    desc.backendInterface                  = bsdfPass->brixelizerParams()->backendInterface;

    ffxCheck(ffxBrixelizerGIContextCreate(&_brixelizerGIContext, &desc));
}

void BGIPass::destroyContext(gerium_renderer_t renderer) {
    gerium_renderer_wait_ffx_jobs(renderer);
    ffxBrixelizerGIContextDestroy(&_brixelizerGIContext);
}

void SkyDomeGenPass::render(gerium_frame_graph_t frameGraph,
                            gerium_renderer_t renderer,
                            gerium_command_buffer_t commandBuffer,
                            gerium_uint32_t worker,
                            gerium_uint32_t totalWorkers) {
    const auto lat  = glm::radians(35.0f);
    const auto decl = glm::radians(23.0f + (17.0f / 60.0f));
    const auto hour = glm::radians((settings().CurrentHour - 12.0f) * 15.0f);

    const auto sinH = glm::sin(lat) * glm::sin(decl) + glm::cos(lat) * glm::cos(decl) * glm::cos(hour);
    const auto cosH = glm::sqrt(1.0f - sinH * sinH);
    const auto sinA = glm::cos(decl) * glm::sin(hour) / cosH;
    const auto cosA = (sinH * glm::sin(lat) - glm::sin(decl)) / (cosH * glm::cos(decl));

    const auto x = cosA * cosH;
    const auto y = sinH;
    const auto z = sinA * cosH;

    const auto dir = glm::normalize(glm::vec3(x, y, z));

    _data.sunDirection    = glm::vec4(dir, 1.0f);
    _data.rayleigh        = 0.09f;
    _data.turbidity       = 1.5f;
    _data.mieCoefficient  = 0.002f;
    _data.luminance       = 15.0f;
    _data.mieDirectionalG = 0.75f;

    auto data = gerium_renderer_map_buffer(renderer, _skyData, 0, sizeof(SkyData));
    memcpy(data, &_data, sizeof(SkyData));
    gerium_renderer_unmap_buffer(renderer, _skyData);

    const auto groupSize = getGroupCount(512, SKY_GROUP_SIZE);
    gerium_command_buffer_bind_technique(commandBuffer, _technique);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet, 0);
    gerium_command_buffer_dispatch(commandBuffer, groupSize, groupSize, 6);
}

void SkyDomeGenPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    std::filesystem::path appDir = gerium_file_get_app_dir();

    gerium_texture_info_t info{};
    info.width   = kSkySize;
    info.height  = kSkySize;
    info.depth   = 1;
    info.mipmaps = 1;
    info.layers  = 6;
    info.format  = GERIUM_FORMAT_R16G16B16A16_SFLOAT;
    info.type    = GERIUM_TEXTURE_TYPE_CUBE;
    info.name    = "skydome_env";

    _skyDome       = application()->resourceManager().createTexture(info, nullptr);
    _technique     = application()->resourceManager().loadTechnique((appDir / "techniques" / "skydome.yaml").string());
    _descriptorSet = application()->resourceManager().createDescriptorSet("", true);
    _skyData       = application()->resourceManager().createBuffer(
        GERIUM_BUFFER_USAGE_UNIFORM_BIT, true, "sky_data", nullptr, sizeof(SkyData));

    gerium_renderer_texture_sampler(renderer,
                                    _skyDome,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_ADDRESS_MODE_REPEAT,
                                    GERIUM_REDUCTION_MODE_WEIGHTED_AVERAGE);

    gerium_frame_graph_add_texture(frameGraph, info.name, _skyDome);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 0, _skyData);
    gerium_renderer_bind_resource(renderer, _descriptorSet, 1, info.name, false);
}

void SkyDomeGenPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
    _skyData       = nullptr;
    _skyDome       = nullptr;
    _technique     = nullptr;
}

void SkyDomePrefilteredPass::render(gerium_frame_graph_t frameGraph,
                                    gerium_renderer_t renderer,
                                    gerium_command_buffer_t commandBuffer,
                                    gerium_uint32_t worker,
                                    gerium_uint32_t totalWorkers) {
    gerium_command_buffer_bind_technique(commandBuffer, _technique);
    for (gerium_uint32_t mip = 0; mip < kMips; ++mip) {
        const auto mipSize   = static_cast<int32_t>(kSkySize * std::pow(0.5, mip));
        const auto groupSize = getGroupCount(mipSize, SKY_GROUP_SIZE);
        gerium_command_buffer_barrier_texture_write(commandBuffer, _skyDomeMips[mip]);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSets[mip], 0);
        gerium_command_buffer_dispatch(commandBuffer, groupSize, groupSize, 6);
        gerium_command_buffer_barrier_texture_read(commandBuffer, _skyDomeMips[mip]);
    }
}

void SkyDomePrefilteredPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    assert(kMips == calcMipLevels(kSkySize, kSkySize));

    std::filesystem::path appDir = gerium_file_get_app_dir();

    gerium_texture_info_t info{};
    info.width   = kSkySize;
    info.height  = kSkySize;
    info.depth   = 1;
    info.mipmaps = kMips;
    info.layers  = 6;
    info.format  = GERIUM_FORMAT_R16G16B16A16_SFLOAT;
    info.type    = GERIUM_TEXTURE_TYPE_CUBE;
    info.name    = "skydome_prefiltered_env";

    _skyDomePrefiltered = application()->resourceManager().createTexture(info, nullptr);
    _technique = application()->resourceManager().loadTechnique((appDir / "techniques" / "skydome.yaml").string());

    gerium_renderer_texture_sampler(renderer,
                                    _skyDomePrefiltered,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_ADDRESS_MODE_REPEAT,
                                    GERIUM_REDUCTION_MODE_WEIGHTED_AVERAGE);

    gerium_frame_graph_add_texture(frameGraph, info.name, _skyDomePrefiltered);

    for (gerium_uint16_t mip = 0; mip < info.mipmaps; ++mip) {
        uint32_t mipSize = static_cast<uint32_t>(kSkySize * std::pow(0.5, mip));
        float roughness  = (float) mip / (float) (info.mipmaps - 1);

        SkyPrefilteredData data{};
        data.invSize.x = 1.0f / mipSize;

        for (gerium_uint32_t i = 0; i < std::size(data.sampleDirections); ++i) {
            auto& sample = data.sampleDirections[i];

            const auto xi = hammersley(i, std::size(data.sampleDirections));
            const float a = roughness * roughness;

            const float phi      = 2.0f * M_PI * xi.x;
            const float cosTheta = std::sqrt((1.0f - xi.y) / (1.0f + (a * a - 1.0f) * xi.y));
            const float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

            sample.x = std::cos(phi) * sinTheta;
            sample.y = std::sin(phi) * sinTheta;
            sample.z = cosTheta;
            sample.w = 0.0f;
        }

        _descriptorSets[mip] = application()->resourceManager().createDescriptorSet("", true);
        _skyPrefilteredDatas[mip] =
            application()->resourceManager().createBuffer(GERIUM_BUFFER_USAGE_UNIFORM_BIT,
                                                          false,
                                                          "sky_prefiltered_data_" + std::to_string(mip),
                                                          &data,
                                                          sizeof(SkyPrefilteredData));

        _skyDomeMips[mip] =
            application()->resourceManager().createTextureView("skydome_prefiltered_env_mip_" + std::to_string(mip),
                                                               _skyDomePrefiltered,
                                                               GERIUM_TEXTURE_TYPE_CUBE,
                                                               mip,
                                                               1,
                                                               0,
                                                               6);

        gerium_renderer_bind_buffer(renderer, _descriptorSets[mip], 0, _skyPrefilteredDatas[mip]);
        gerium_renderer_bind_resource(renderer, _descriptorSets[mip], 1, "skydome_env", false);
        gerium_renderer_bind_texture(renderer, _descriptorSets[mip], 2, 0, _skyDomeMips[mip]);
    }
}

void SkyDomePrefilteredPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _technique           = {};
    _skyDomePrefiltered  = {};
    _skyPrefilteredDatas = {};
    _skyDomeMips         = {};
    _descriptorSets      = {};
}

void SkydomePass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    auto camera = (settings().DebugCamera && settings().MoveDebugCamera) ? application()->getDebugCamera()
                                                                         : application()->getCamera();
    gerium_command_buffer_bind_technique(commandBuffer, _technique);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet, 1);
    gerium_command_buffer_draw(commandBuffer, 0, 3, 0, 1);
}

void SkydomePass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    std::filesystem::path appDir = gerium_file_get_app_dir();

    _technique     = application()->resourceManager().loadTechnique((appDir / "techniques" / "skydome.yaml").string());
    _descriptorSet = application()->resourceManager().createDescriptorSet("", true);

    gerium_renderer_bind_resource(renderer, _descriptorSet, 0, "skydome_env", false);
}

void SkydomePass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
    _technique     = nullptr;
}

Application::Application() {
    check(gerium_logger_create("example", &_logger));
}

Application::~Application() {
    if (_application) {
        gerium_application_destroy(_application);
        _application = nullptr;
    }
    if (_logger) {
        gerium_logger_destroy(_logger);
        _logger = nullptr;
    }
}

void Application::run(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) {
    try {
        check(gerium_application_create(title, width, height, &_application));
        gerium_application_set_background_wait(_application, true);
        gerium_application_set_frame_func(_application, frame, (gerium_data_t) this);
        gerium_application_set_state_func(_application, state, (gerium_data_t) this);
        auto result = gerium_application_run(_application);
        if (_error) {
            std::rethrow_exception(_error);
        }
        check(result);
    } catch (const std::exception& exc) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_FATAL, exc.what());
    } catch (...) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_FATAL, "unknown error");
    }
}

void Application::createScene() {
    _cluster = loadCluster("bistro");

    DrawData drawData{};
    drawData.drawCount = glm::uint(_cluster.instanceCount);
    drawData.lodTarget = 0;
    _drawData =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_UNIFORM_BIT, false, "draw_data", &drawData, sizeof(drawData));
}

Cluster Application::loadCluster(std::string_view name) {
    std::filesystem::path appDir = gerium_file_get_app_dir();

    auto fileName = (appDir / "models" / std::filesystem::path(name).replace_extension("cluster")).string();

    const auto id = '_' + std::string(name.data(), name.length());

    gerium_file_t file;
    check(gerium_file_open(fileName.c_str(), true, &file));
    deferred(gerium_file_destroy(file));

    auto data                 = (const gerium_uint32_t*) gerium_file_map(file);
    auto verticesSize         = *data++;
    auto meshletsSize         = *data++;
    auto meshesSize           = *data++;
    auto simpleMeshesSize     = *data++;
    auto vertexIndecesSize    = *data++;
    auto primitiveIndicesSize = *data++;
    auto shadowIndicesSize    = *data++;
    auto instancesSize        = *data++;

    Cluster result{};
    result.instanceCount = instancesSize / sizeof(Instance);

    const auto vertexCount = verticesSize / sizeof(VertexCompressed);

    auto cluster = (const gerium_uint8_t*) data;

    result.vertices =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, false, "vertices" + id, cluster, verticesSize);

    std::vector<f16vec4> shadowVertices(vertexCount);
    const auto vertices = (VertexCompressed*) cluster;
    const auto halfOne  = meshopt_quantizeHalf(1.0f);
    for (size_t i = 0; i < vertexCount; ++i) {
        shadowVertices[i].x = vertices[i].px;
        shadowVertices[i].y = vertices[i].py;
        shadowVertices[i].z = vertices[i].pz;
        shadowVertices[i].w = halfOne;
    }
    result.shadowVertices = _resourceManager.createBuffer(
        GERIUM_BUFFER_USAGE_STORAGE_BIT, false, "shadow_vertices" + id, shadowVertices.data(), vertexCount * 8);
    cluster += verticesSize;

    result.meshlets =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, false, "meshlets" + id, cluster, meshletsSize);
    cluster += meshletsSize;

    std::vector<MeshCompressed> meshes(meshesSize / sizeof(MeshCompressed));
    memcpy(meshes.data(), cluster, meshesSize);
    cluster += meshesSize;

    result.meshes =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, false, "meshes" + id, meshes.data(), meshesSize);

    std::vector<SimpleMesh> simpleMeshes(simpleMeshesSize / sizeof(SimpleMesh));
    memcpy(simpleMeshes.data(), cluster, simpleMeshesSize);
    cluster += simpleMeshesSize;

    result.vertexIndices = _resourceManager.createBuffer(
        GERIUM_BUFFER_USAGE_STORAGE_BIT, false, "vertex_indices" + id, cluster, vertexIndecesSize);
    cluster += vertexIndecesSize;

    result.primitiveIndices = _resourceManager.createBuffer(
        GERIUM_BUFFER_USAGE_STORAGE_BIT, false, "primitive_indices" + id, cluster, primitiveIndicesSize);
    cluster += primitiveIndicesSize;

    result.shadowIndices = _resourceManager.createBuffer(
        GERIUM_BUFFER_USAGE_STORAGE_BIT, false, "shadow_indices" + id, cluster, shadowIndicesSize);
    cluster += shadowIndicesSize;

    std::vector<Instance> instances(result.instanceCount);
    memcpy(instances.data(), cluster, instancesSize);
    cluster += instancesSize;

    for (auto& instance : instances) {
        auto baseName = std::string(name.data(), name.length()) + '_' + std::to_string(instance.baseTexture) + "_base";
        auto metalnessName =
            std::string(name.data(), name.length()) + '_' + std::to_string(instance.baseTexture) + "_metalness";
        auto normalName =
            std::string(name.data(), name.length()) + '_' + std::to_string(instance.baseTexture) + "_normal";
        auto baseFullName =
            (appDir / "models" / "textures" / std::filesystem::path(baseName).replace_extension("ktx2")).string();
        auto metalnessFullName =
            (appDir / "models" / "textures" / std::filesystem::path(metalnessName).replace_extension("ktx2")).string();
        auto normalFullName =
            (appDir / "models" / "textures" / std::filesystem::path(normalName).replace_extension("ktx2")).string();

        _textures.push_back(_resourceManager.loadTexture(baseFullName));
        gerium_renderer_texture_sampler(_renderer,
                                        _textures.back(),
                                        GERIUM_FILTER_LINEAR,
                                        GERIUM_FILTER_LINEAR,
                                        GERIUM_FILTER_LINEAR,
                                        GERIUM_ADDRESS_MODE_REPEAT,
                                        GERIUM_ADDRESS_MODE_REPEAT,
                                        GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                        GERIUM_REDUCTION_MODE_WEIGHTED_AVERAGE);
        instance.baseTexture = ((gerium_texture_h) _textures.back()).index;

        _textures.push_back(_resourceManager.loadTexture(metalnessFullName));
        gerium_renderer_texture_sampler(_renderer,
                                        _textures.back(),
                                        GERIUM_FILTER_LINEAR,
                                        GERIUM_FILTER_LINEAR,
                                        GERIUM_FILTER_LINEAR,
                                        GERIUM_ADDRESS_MODE_REPEAT,
                                        GERIUM_ADDRESS_MODE_REPEAT,
                                        GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                        GERIUM_REDUCTION_MODE_WEIGHTED_AVERAGE);
        instance.metalnessTexture = ((gerium_texture_h) _textures.back()).index;

        _textures.push_back(_resourceManager.loadTexture(normalFullName));
        gerium_renderer_texture_sampler(_renderer,
                                        _textures.back(),
                                        GERIUM_FILTER_LINEAR,
                                        GERIUM_FILTER_LINEAR,
                                        GERIUM_FILTER_LINEAR,
                                        GERIUM_ADDRESS_MODE_REPEAT,
                                        GERIUM_ADDRESS_MODE_REPEAT,
                                        GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                        GERIUM_REDUCTION_MODE_WEIGHTED_AVERAGE);
        instance.normalTexture = ((gerium_texture_h) _textures.back()).index;
    }

    result.instances = _resourceManager.createBuffer(
        GERIUM_BUFFER_USAGE_STORAGE_BIT, false, "instances" + id, instances.data(), instancesSize);

    result.descriptorSet = _resourceManager.createDescriptorSet("descriptor_set" + id, true);
    gerium_renderer_bind_buffer(_renderer, result.descriptorSet, 0, result.vertices);
    gerium_renderer_bind_buffer(_renderer, result.descriptorSet, 1, result.meshlets);
    gerium_renderer_bind_buffer(_renderer, result.descriptorSet, 2, result.vertexIndices);
    gerium_renderer_bind_buffer(_renderer, result.descriptorSet, 3, result.primitiveIndices);
    gerium_renderer_bind_buffer(_renderer, result.descriptorSet, 4, result.meshes);
    gerium_renderer_bind_buffer(_renderer, result.descriptorSet, 5, result.instances);

    auto bsdfPass = getPass<BSDFPass>();

    uint32_t shadowIndex = bsdfPass->registerBuffer(_renderer, result.shadowIndices);
    uint32_t vertexIndex = bsdfPass->registerBuffer(_renderer, result.shadowVertices);

    for (const auto& instance : instances) {
        const auto& transform  = instance.world;
        const auto& mesh       = meshes[instance.mesh];
        const auto& simpleMesh = simpleMeshes[instance.mesh];
        const auto aabbMin     = glm::vec4(meshopt_dequantizeHalf(mesh.bboxMin[0]),
                                       meshopt_dequantizeHalf(mesh.bboxMin[1]),
                                       meshopt_dequantizeHalf(mesh.bboxMin[2]),
                                       1.0f);
        const auto aabbMax     = glm::vec4(meshopt_dequantizeHalf(mesh.bboxMax[0]),
                                       meshopt_dequantizeHalf(mesh.bboxMax[1]),
                                       meshopt_dequantizeHalf(mesh.bboxMax[2]),
                                       1.0f);
        const auto extents     = aabbMax - aabbMin;

        const glm::vec4 aabbCorners[8] = {
            aabbMin + glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
            aabbMin + glm::vec4(extents.x, 0.0f, 0.0f, 0.0f),
            aabbMin + glm::vec4(0.0f, 0.0f, extents.z, 0.0f),
            aabbMin + glm::vec4(extents.x, 0.0f, extents.z, 0.0f),
            aabbMin + glm::vec4(0.0f, extents.y, 0.0f, 0.0f),
            aabbMin + glm::vec4(extents.x, extents.y, 0.0f, 0.0f),
            aabbMin + glm::vec4(0.0f, extents.y, extents.z, 0.0f),
            aabbMin + glm::vec4(extents.x, extents.y, extents.z, 0.0f),
        };

        auto minExtents = glm::vec4(INFINITY, INFINITY, INFINITY, INFINITY);
        auto maxExtents = glm::vec4(-INFINITY, -INFINITY, -INFINITY, -INFINITY);

        for (uint32_t i = 0; i < 8; i++) {
            const auto point = transform * aabbCorners[i];
            minExtents       = glm::min(minExtents, point);
            maxExtents       = glm::max(maxExtents, point);
        }

        FfxBrixelizerInstanceDescription desc{};
        for (int i = 0; i < 3; ++i) {
            desc.aabb.min[i] = minExtents[i];
            desc.aabb.max[i] = maxExtents[i];
        }
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 4; ++c) {
                desc.transform[r * 4 + c] = glm::column(transform, c)[r];
            }
        }

        desc.maxCascade         = 0;
        desc.indexFormat        = FFX_INDEX_TYPE_UINT32;
        desc.indexBuffer        = shadowIndex;
        desc.indexBufferOffset  = simpleMesh.primitiveOffset * 4;
        desc.triangleCount      = simpleMesh.primitiveCount;
        desc.vertexBuffer       = vertexIndex;
        desc.vertexStride       = sizeof(f16vec4);
        desc.vertexBufferOffset = simpleMesh.vertexOffset * sizeof(f16vec4);
        desc.vertexCount        = simpleMesh.vertexCount;
        desc.vertexFormat       = FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT;
        desc.flags              = FFX_BRIXELIZER_INSTANCE_FLAG_NONE;
        bsdfPass->createInstance(desc);
    }

    return result;
}

void Application::initialize() {
    constexpr auto debug =
#ifdef NDEBUG
        false;
#else
        true;
#endif

    gerium_application_get_size(_application, &_width, &_height);
    _prevWidth  = _width;
    _prevHeight = _height;
    _invWidth   = 1.0f / _width;
    _invHeight  = 1.0f / _height;

    check(gerium_renderer_create(_application,
                                 GERIUM_FEATURE_BINDLESS_BIT | GERIUM_FEATURE_MESH_SHADER_BIT |
                                     GERIUM_FEATURE_8_BIT_STORAGE_BIT | GERIUM_FEATURE_16_BIT_STORAGE_BIT,
                                 GERIUM_VERSION_ENCODE(1, 0, 0),
                                 debug,
                                 &_renderer));
    gerium_renderer_set_profiler_enable(_renderer, true);

    _bindlessSupported   = gerium_renderer_get_enabled_features(_renderer) & GERIUM_FEATURE_BINDLESS_BIT;
    _meshShaderSupported = gerium_renderer_get_enabled_features(_renderer) & GERIUM_FEATURE_MESH_SHADER_BIT;

    check(gerium_profiler_create(_renderer, &_profiler));
    check(gerium_frame_graph_create(_renderer, &_frameGraph));

    _resourceManager.create(_renderer, _frameGraph);

    addPass<PresentPass>();
    addPass<GBufferPass>(false);
    addPass<CullingPass>(false);
    addPass<IndirectPass>(false);
    addPass<DepthPyramidPass>();
    addPass<CullingPass>(true);
    addPass<IndirectPass>(true);
    addPass<GBufferPass>(true);
    addPass<DebugOcclusionPass>();
    addPass<LightPass>();
    addPass<DebugLinePass>();
    addPass<BSDFPass>();
    addPass<BGIPass>();
    addPass<SkyDomeGenPass>();
    addPass<SkyDomePrefilteredPass>();
    addPass<SkydomePass>();
    for (auto& renderPass : _renderPasses) {
        renderPass->registerResources(_frameGraph, _renderer);
    }

    std::filesystem::path appDir = gerium_file_get_app_dir();
    _resourceManager.loadFrameGraph((appDir / "frame-graphs" / "main.yaml").string());
    _resourceManager.loadFrameGraph((appDir / "frame-graphs" / "skydome.yaml").string());
    check(gerium_frame_graph_compile(_frameGraph));

    _baseTechnique = _resourceManager.loadTechnique((appDir / "techniques" / "base.yaml").string());

    for (size_t i = 0; i < _noiseTextures.size(); ++i) {
        auto fullPath     = appDir / "textures" / "noise" / ("LDR_RG01_" + std::to_string(i) + ".png");
        _noiseTextures[i] = _resourceManager.loadTexture(fullPath.string());
    }
    _brdfLut = _resourceManager.loadTexture((appDir / "textures" / "brdf" / "BRDFLut.png").string());

    createScene();

    _texturesSet = _resourceManager.createDescriptorSet("texture_bindless", true);
    for (const auto& texture : _textures) {
        gerium_texture_h handle = texture;
        gerium_renderer_bind_texture(_renderer, _texturesSet, 0, handle.index, handle);
    }

    _camera      = Camera(_application, _resourceManager);
    _debugCamera = Camera(_application, _resourceManager);

    for (auto& renderPass : _renderPasses) {
        renderPass->initialize(_frameGraph, _renderer);
    }
}

void Application::uninitialize() {
    if (_renderer) {
        _cluster     = {};
        _texturesSet = nullptr;
        _textures.clear();

        _baseTechnique = nullptr;

        for (auto it = _renderPasses.rbegin(); it != _renderPasses.rend(); ++it) {
            (*it)->uninitialize(_frameGraph, _renderer);
        }

        _resourceManager.destroy();

        if (_frameGraph) {
            gerium_frame_graph_destroy(_frameGraph);
            _frameGraph = nullptr;
        }
        if (_profiler) {
            gerium_profiler_destroy(_profiler);
            _profiler = nullptr;
        }
        gerium_renderer_destroy(_renderer);
        _renderer = nullptr;
    }
}

void Application::pollInput(gerium_uint64_t elapsedMs) {
    bool swapFullscreen = false;
    bool showCursor     = gerium_application_is_show_cursor(_application);
    bool invY           = gerium_application_get_platform(_application) == GERIUM_RUNTIME_PLATFORM_MAC_OS;

    auto move = 1.0f;
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_SHIFT_LEFT) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_SHIFT_RIGHT)) {
        move *= 2.0f;
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_CONTROL_LEFT) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_CONTROL_RIGHT)) {
        move /= 2.0f;
    }

    gerium_event_t event;
    gerium_float32_t pitch = 0.0f;
    gerium_float32_t yaw   = 0.0f;
    gerium_float32_t zoom  = 0.0f;
    while (gerium_application_poll_events(_application, &event)) {
        if (event.type == GERIUM_EVENT_TYPE_KEYBOARD) {
            if (event.keyboard.scancode == GERIUM_SCANCODE_ENTER && event.keyboard.state == GERIUM_KEY_STATE_RELEASED &&
                (event.keyboard.modifiers & GERIUM_KEY_MOD_LALT)) {
                swapFullscreen = true;
            } else if (event.keyboard.scancode == GERIUM_SCANCODE_ESCAPE &&
                       event.keyboard.state == GERIUM_KEY_STATE_RELEASED) {
                showCursor = true;
            }
        } else if (event.type == GERIUM_EVENT_TYPE_MOUSE) {
            constexpr auto buttonsDown = GERIUM_MOUSE_BUTTON_LEFT_DOWN | GERIUM_MOUSE_BUTTON_RIGHT_DOWN |
                                         GERIUM_MOUSE_BUTTON_MIDDLE_DOWN | GERIUM_MOUSE_BUTTON_4_DOWN |
                                         GERIUM_MOUSE_BUTTON_5_DOWN;
            if (event.mouse.buttons & buttonsDown) {
                showCursor = false;
            }
            if (!gerium_application_is_show_cursor(_application) ||
                gerium_application_get_platform(_application) == GERIUM_RUNTIME_PLATFORM_ANDROID) {
                const auto delta = 1.0f;
                pitch += event.mouse.raw_delta_y * (invY ? delta : -delta);
                yaw += event.mouse.raw_delta_x * -delta;
                zoom += event.mouse.wheel_vertical * move * -0.1f;
            }
        }
    }

    auto camera = (settings().DebugCamera && settings().MoveDebugCamera) ? getDebugCamera() : getCamera();
    camera->rotate(pitch, yaw, 1.0f);
    camera->zoom(zoom, 1.0f);

    auto elapsed = (gerium_float32_t) elapsedMs;
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_A) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_ARROW_LEFT)) {
        camera->move(Camera::Right, move, elapsed);
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_D) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_ARROW_RIGHT)) {
        camera->move(Camera::Right, -move, elapsed);
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_W) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_ARROW_UP)) {
        camera->move(Camera::Forward, move, elapsed);
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_S) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_ARROW_DOWN)) {
        camera->move(Camera::Forward, -move, elapsed);
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_SPACE) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_PAGE_UP)) {
        camera->move(Camera::Up, move, elapsed);
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_C) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_PAGE_DOWN)) {
        camera->move(Camera::Up, -move, elapsed);
    }

    if (swapFullscreen) {
        gerium_application_fullscreen(_application, !gerium_application_is_fullscreen(_application), 0, nullptr);
    }

    if (gerium_application_get_platform(_application) != GERIUM_RUNTIME_PLATFORM_ANDROID) {
        gerium_application_show_cursor(_application, showCursor);
    }
}

void Application::frame(gerium_uint64_t elapsedMs) {
    pollInput(elapsedMs);

    // disable debug passes from frame graph if debug camera is disabled
    // gerium_frame_graph_enable_node(_frameGraph, "debug_occlusion_pass", _settings.DebugCamera);
    // gerium_frame_graph_enable_node(_frameGraph, "debug_line_pass", _settings.DebugCamera);

    if (gerium_renderer_new_frame(_renderer) == GERIUM_RESULT_SKIP_FRAME) {
        return;
    }

    _prevWidth  = _width;
    _prevHeight = _height;
    gerium_application_get_size(_application, &_width, &_height);

    if (_prevWidth != _width || _prevHeight != _height) {
        _invWidth  = 1.0f / _width;
        _invHeight = 1.0f / _height;
    }

    _resourceManager.update(elapsedMs);

    auto sunSpeed = 0.0025f;
    if (settings().CurrentHour < settings().Hour) {
        settings().CurrentHour += elapsedMs * sunSpeed;
        if (settings().CurrentHour > settings().Hour) {
            settings().CurrentHour = settings().Hour;
        }
    } else if (settings().CurrentHour > settings().Hour) {
        settings().CurrentHour -= elapsedMs * sunSpeed;
        if (settings().CurrentHour < settings().Hour) {
            settings().CurrentHour = settings().Hour;
        }
    }

    getDebugCamera()->update(settings().Output);
    getCamera()->update(settings().Output);

    check(gerium_renderer_render(_renderer, _frameGraph));
    check(gerium_renderer_present(_renderer));
}

void Application::state(gerium_application_state_t state) {
    const auto stateStr = magic_enum::enum_name(state);
    gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_DEBUG, stateStr.data());

    switch (state) {
        case GERIUM_APPLICATION_STATE_INITIALIZE:
            initialize();
            break;
        case GERIUM_APPLICATION_STATE_UNINITIALIZE:
            uninitialize();
            break;
        case GERIUM_APPLICATION_STATE_GOT_FOCUS:
        case GERIUM_APPLICATION_STATE_LOST_FOCUS:
        case GERIUM_APPLICATION_STATE_VISIBLE:
        case GERIUM_APPLICATION_STATE_INVISIBLE:
            gerium_application_show_cursor(_application, true);
            break;
        default:
            break;
    }
}

gerium_bool_t Application::frame(gerium_application_t application, gerium_data_t data, gerium_uint64_t elapsedMs) {
    auto app = (Application*) data;
    return app->cppCallWrap([app, elapsedMs]() {
        app->frame(elapsedMs);
    });
}

gerium_bool_t Application::state(gerium_application_t application,
                                 gerium_data_t data,
                                 gerium_application_state_t state) {
    auto app = (Application*) data;
    return app->cppCallWrap([app, state]() {
        app->state(state);
    });
}

gerium_uint32_t Application::prepare(gerium_frame_graph_t frameGraph,
                                     gerium_renderer_t renderer,
                                     gerium_uint32_t maxWorkers,
                                     gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    return renderPass->application()->cppCallWrap([renderPass, frameGraph, renderer, maxWorkers]() {
        return renderPass->prepare(frameGraph, renderer, maxWorkers);
    });
}

gerium_bool_t Application::resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer, gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    return renderPass->application()->cppCallWrap([renderPass, frameGraph, renderer]() {
        renderPass->resize(frameGraph, renderer);
    });
}

gerium_bool_t Application::render(gerium_frame_graph_t frameGraph,
                                  gerium_renderer_t renderer,
                                  gerium_command_buffer_t commandBuffer,
                                  gerium_uint32_t worker,
                                  gerium_uint32_t totalWorkers,
                                  gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    return renderPass->application()->cppCallWrap(
        [renderPass, frameGraph, renderer, commandBuffer, worker, totalWorkers]() {
        renderPass->render(frameGraph, renderer, commandBuffer, worker, totalWorkers);
    });
}
