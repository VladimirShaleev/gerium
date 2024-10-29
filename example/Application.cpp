#include "Application.hpp"

void DepthPyramidPass::render(gerium_frame_graph_t frameGraph,
                              gerium_renderer_t renderer,
                              gerium_command_buffer_t commandBuffer,
                              gerium_uint32_t worker,
                              gerium_uint32_t totalWorkers) {
    gerium_texture_h depth;
    gerium_renderer_get_texture(renderer, "depth", false, false, &depth);
    gerium_renderer_texture_sampler(renderer,
                                    depth,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_FILTER_NEAREST,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_REDUCTION_MODE_MIN);

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
    gerium_renderer_texture_sampler(renderer,
                                    _depthPyramid,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_FILTER_LINEAR,
                                    GERIUM_FILTER_NEAREST,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE,
                                    GERIUM_REDUCTION_MODE_MIN);

    gerium_frame_graph_add_texture(frameGraph, "depth_pyramid", _depthPyramid);

    for (gerium_uint16_t m = 0; m < _depthPyramidMipLevels; ++m) {
        auto levelWidth  = std::max(1, _depthPyramidWidth >> m);
        auto levelHeight = std::max(1, _depthPyramidHeight >> m);
        auto imageSize   = glm::vec2(levelWidth, levelHeight);
        _imageSizes[m]   = application()->resourceManager().createBuffer(
            GERIUM_BUFFER_USAGE_UNIFORM_BIT, false, "", &imageSize.x, sizeof(imageSize), 0);

        auto name = "depth_pyramid_mip_" + std::to_string(m);

        _depthPyramidMips[m] = application()->resourceManager().createTextureView(name, _depthPyramid, m, 1, 0);
        _descriptorSets[m]   = application()->resourceManager().createDescriptorSet("", true, 0);

        if (m == 0) {
            gerium_renderer_bind_resource(renderer, _descriptorSets[m], 0, "depth");
            gerium_renderer_bind_texture(renderer, _descriptorSets[m], 1, 0, _depthPyramidMips[m]);
            gerium_renderer_bind_buffer(renderer, _descriptorSets[m], 2, _imageSizes[m]);
        } else {
            gerium_renderer_bind_texture(renderer, _descriptorSets[m], 0, 0, _depthPyramidMips[m - 1]);
            gerium_renderer_bind_texture(renderer, _descriptorSets[m], 1, 0, _depthPyramidMips[m]);
            gerium_renderer_bind_buffer(renderer, _descriptorSets[m], 2, _imageSizes[m]);
        }
    }
}

void CullingPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    auto groupSize = getGroupCount(application()->cluster().instanceCount, 64U);
    auto camera    = application()->getCamera();
    gerium_buffer_h commandCount;
    check(gerium_renderer_get_buffer(renderer, !_latePass ? "command_count" : "command_count_late", 1, &commandCount));
    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet0, GLOBAL_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet1, MESH_DATA_SET);
    gerium_command_buffer_fill_buffer(commandBuffer, commandCount, 0, 4, 0);
    gerium_command_buffer_dispatch(commandBuffer, groupSize, 1, 1);
}

void CullingPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    auto& cluster   = application()->cluster();
    _descriptorSet0 = application()->resourceManager().createDescriptorSet("", true);
    _descriptorSet1 = application()->resourceManager().createDescriptorSet("", true);
    gerium_renderer_bind_buffer(renderer, _descriptorSet0, 0, application()->drawData());
    gerium_renderer_bind_resource(renderer, _descriptorSet0, 1, !_latePass ? "command_count" : "command_count_late");
    gerium_renderer_bind_resource(renderer, _descriptorSet0, 2, "commands");
    gerium_renderer_bind_resource(renderer, _descriptorSet0, 3, "visibility");
    if (_latePass) {
        gerium_renderer_bind_resource(renderer, _descriptorSet0, 4, "depth_pyramid");
    }
    gerium_renderer_bind_buffer(renderer, _descriptorSet1, 0, cluster.instancesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet1, 1, cluster.meshesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet1, 2, cluster.meshletsBuffer);
}

void CullingPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet1 = nullptr;
    _descriptorSet0 = nullptr;
}

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
    _descriptorSet = application()->resourceManager().createDescriptorSet("", true);
    gerium_renderer_bind_resource(renderer, _descriptorSet, 0, !_latePass ? "command_count" : "command_count_late");
    gerium_renderer_bind_resource(renderer, _descriptorSet, 1, "commands");
}

void IndirectPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
}

void GBufferPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    auto camera = application()->getCamera();
    gerium_buffer_h commandCount;
    check(gerium_renderer_get_buffer(renderer, !_latePass ? "command_count" : "command_count_late", 0, &commandCount));
    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet, GLOBAL_DATA_SET);
    gerium_command_buffer_draw_mesh_tasks_indirect(commandBuffer, commandCount, 4, 1, 12);
}

void GBufferPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    auto& cluster  = application()->cluster();
    _descriptorSet = application()->resourceManager().createDescriptorSet("", true);

    gerium_renderer_bind_resource(renderer, _descriptorSet, 0, "commands");
    gerium_renderer_bind_resource(renderer, _descriptorSet, 1, "meshlet_visibility");
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 2, cluster.instancesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 3, cluster.meshesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 4, cluster.meshletsBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 5, cluster.vertexIndicesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 6, cluster.primitiveIndicesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 7, cluster.verticesBuffer);
    gerium_renderer_bind_resource(renderer, _descriptorSet, 8, "depth_pyramid");
}

void GBufferPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
}

void PresentPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    static bool drawProfiler = false;

    auto ds        = application()->resourceManager().createDescriptorSet("");
    auto& settings = application()->settings();
    gerium_renderer_bind_resource(renderer, ds, 0, application()->settings().DebugCamera ? "debug_meshlet" : "color");

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
        ImGui::Checkbox("Show profiler", &drawProfiler);
        ImGui::Checkbox("Debug camera", &settings.DebugCamera);
        ImGui::Checkbox("Move debug camera", &settings.MoveDebugCamera);
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
    check(gerium_renderer_get_buffer(renderer, "command_count_late", 0, &commandCount));
    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet, GLOBAL_DATA_SET);
    gerium_command_buffer_draw_mesh_tasks_indirect(commandBuffer, commandCount, 4, 1, 12);
}

void DebugOcclusionPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    auto& cluster  = application()->cluster();
    _descriptorSet = application()->resourceManager().createDescriptorSet("", true);

    gerium_renderer_bind_resource(renderer, _descriptorSet, 0, "commands");
    gerium_renderer_bind_resource(renderer, _descriptorSet, 1, "meshlet_visibility");
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 2, cluster.instancesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 3, cluster.meshesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 4, cluster.meshletsBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 5, cluster.vertexIndicesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 6, cluster.primitiveIndicesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 7, cluster.verticesBuffer);
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
    gerium_renderer_bind_resource(renderer, _descriptorSet, 0, "debug_meshlet");

    _maxPoints = 48;
    _vertices  = application()->resourceManager().createBuffer(
        GERIUM_BUFFER_USAGE_VERTEX_BIT, true, "lines_vertices", nullptr, sizeof(glm::vec3) * _maxPoints);
}

void DebugLinePass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
    _vertices      = nullptr;
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
        check(gerium_application_run(_application));
        if (_error) {
            std::rethrow_exception(_error);
        }
    } catch (const std::exception& exc) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_FATAL, exc.what());
    } catch (...) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_FATAL, "unknown error");
    }
}

void Application::addPass(RenderPass& renderPass) {
    renderPass.setApplication(this);
    _renderPasses.push_back(&renderPass);

    gerium_render_pass_t pass{ prepare, resize, render };
    gerium_frame_graph_add_pass(_frameGraph, renderPass.name().c_str(), &pass, &renderPass);
}

size_t appendMeshlets(Cluster& cluster,
                      const VertexNonCompressed* vertices,
                      size_t verticesOffset,
                      size_t verticesCount,
                      const std::vector<uint32_t>& indices) {
    constexpr size_t maxVertices  = MESH_MAX_VERTICES;
    constexpr size_t maxTriangles = MESH_MAX_PRIMITIVES;
    constexpr float coneWeight    = 0.5f;

    std::vector<meshopt_Meshlet> meshlets(meshopt_buildMeshletsBound(indices.size(), maxVertices, maxTriangles));
    std::vector<unsigned int> meshletVertices(meshlets.size() * maxVertices);
    std::vector<unsigned char> meshletTriangles(meshlets.size() * maxTriangles * 3);

    meshlets.resize(meshopt_buildMeshlets(meshlets.data(),
                                          meshletVertices.data(),
                                          meshletTriangles.data(),
                                          indices.data(),
                                          indices.size(),
                                          &vertices[0].px,
                                          verticesCount,
                                          sizeof(VertexNonCompressed),
                                          maxVertices,
                                          maxTriangles,
                                          coneWeight));

    for (auto& meshlet : meshlets) {
        meshopt_optimizeMeshlet(&meshletVertices[meshlet.vertex_offset],
                                &meshletTriangles[meshlet.triangle_offset],
                                meshlet.triangle_count,
                                meshlet.vertex_count);

        const auto vertexOffset    = cluster.vertexIndices.size();
        const auto primitiveOffset = cluster.primitiveIndices.size();

        for (unsigned int i = 0; i < meshlet.vertex_count; ++i) {
            cluster.vertexIndices.push_back(verticesOffset + meshletVertices[meshlet.vertex_offset + i]);
        }

        for (unsigned int i = 0; i < meshlet.triangle_count * 3; ++i) {
            cluster.primitiveIndices.push_back(meshletTriangles[meshlet.triangle_offset + i]);
        }

        const auto bounds = meshopt_computeMeshletBounds(&meshletVertices[meshlet.vertex_offset],
                                                         &meshletTriangles[meshlet.triangle_offset],
                                                         meshlet.triangle_count,
                                                         &vertices[0].px,
                                                         verticesCount,
                                                         sizeof(VertexNonCompressed));

        MeshletNonCompressed meshletInfo = {};
        meshletInfo.vertexOffset         = uint32_t(vertexOffset);
        meshletInfo.primitiveOffset      = uint32_t(primitiveOffset);
        meshletInfo.vertexCount          = meshlet.vertex_count;
        meshletInfo.primitiveCount       = meshlet.triangle_count;

        meshletInfo.center[0]   = bounds.center[0];
        meshletInfo.center[1]   = bounds.center[1];
        meshletInfo.center[2]   = bounds.center[2];
        meshletInfo.radius      = bounds.radius;
        meshletInfo.coneAxis[0] = bounds.cone_axis[0];
        meshletInfo.coneAxis[1] = bounds.cone_axis[1];
        meshletInfo.coneAxis[2] = bounds.cone_axis[2];
        meshletInfo.coneCutoff  = bounds.cone_cutoff;

        cluster.meshlets.push_back(meshletInfo);
    }

    return meshlets.size();
}

std::pair<Instance, uint32_t> loadMesh(Cluster& cluster, const aiMesh* mesh) {
    Instance result{};
    uint32_t maxMeshlets = 0;

    auto indexCount = mesh->mNumFaces * 3;
    std::vector<uint32_t> remap(indexCount);
    std::vector<uint32_t> indicesOrigin(indexCount);
    std::vector<VertexNonCompressed> verticesOrigin(mesh->mNumVertices);

    for (int v = 0; v < mesh->mNumVertices; ++v) {
        const auto vertex   = mesh->mVertices[v];
        const auto normal   = mesh->mNormals[v];
        const auto texcoord = mesh->mTextureCoords[0][v];

        auto pos = glm::vec3(vertex.x, vertex.y, vertex.z);
        auto n   = glm::vec3(normal.x, normal.y, normal.z) * 127.0f + 127.5f;
        auto uv  = glm::vec2(texcoord.x, 1.0f - texcoord.y);

        verticesOrigin[v].px = pos.x;
        verticesOrigin[v].py = pos.y;
        verticesOrigin[v].pz = pos.z;
        verticesOrigin[v].nx = normal.x;
        verticesOrigin[v].ny = normal.y;
        verticesOrigin[v].nz = normal.z;
        verticesOrigin[v].tu = uv.x;
        verticesOrigin[v].tv = uv.y;
    }

    for (int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace* face = &mesh->mFaces[i];
        assert(face->mNumIndices == 3);

        indicesOrigin[i * 3 + 0] = face->mIndices[0];
        indicesOrigin[i * 3 + 1] = face->mIndices[1];
        indicesOrigin[i * 3 + 2] = face->mIndices[2];
    }

    auto vertexCount = meshopt_generateVertexRemap(remap.data(),
                                                   indicesOrigin.data(),
                                                   indexCount,
                                                   verticesOrigin.data(),
                                                   verticesOrigin.size(),
                                                   sizeof(VertexNonCompressed));

    auto offsetVertices = cluster.vertices.size();
    cluster.vertices.resize(offsetVertices + vertexCount);
    VertexNonCompressed* vertices = cluster.vertices.data() + offsetVertices;
    std::vector<uint32_t> indices(indexCount);

    meshopt_remapVertexBuffer(
        vertices, verticesOrigin.data(), verticesOrigin.size(), sizeof(VertexNonCompressed), remap.data());
    meshopt_remapIndexBuffer(indices.data(), indicesOrigin.data(), indexCount, remap.data());

    meshopt_optimizeVertexCache(indices.data(), indices.data(), indexCount, vertexCount);
    meshopt_optimizeVertexFetch(
        vertices, indices.data(), indexCount, vertices, vertexCount, sizeof(VertexNonCompressed));

    glm::vec3 center{};
    for (int i = 0; i < vertexCount; ++i) {
        const auto& v = vertices[i];
        center += glm::vec3(v.px, v.py, v.pz);
    }
    center /= float(vertexCount);

    float radius = 0;
    for (int i = 0; i < vertexCount; ++i) {
        const auto& v = vertices[i];
        radius        = glm::max(radius, glm::distance(center, glm::vec3(v.px, v.py, v.pz)));
    }

    result.mesh = glm::uint(cluster.meshes.size());
    cluster.meshes.push_back({});
    float lodError         = 0.0f;
    float normalWeights[3] = { 0.5f, 0.5f, 0.5f };
    auto& meshLods         = cluster.meshes.back();
    meshLods.center[0]     = center.x;
    meshLods.center[1]     = center.y;
    meshLods.center[2]     = center.z;
    meshLods.radius        = radius;
    meshLods.bboxMin[0]    = mesh->mAABB.mMin.x;
    meshLods.bboxMin[1]    = mesh->mAABB.mMin.y;
    meshLods.bboxMin[2]    = mesh->mAABB.mMin.z;
    meshLods.bboxMax[0]    = mesh->mAABB.mMax.x;
    meshLods.bboxMax[1]    = mesh->mAABB.mMax.y;
    meshLods.bboxMax[2]    = mesh->mAABB.mMax.z;

    const auto lodScale = meshopt_simplifyScale(&vertices->px, vertexCount, sizeof(VertexNonCompressed));

    while (meshLods.lodCount < std::size(meshLods.lods)) {
        auto& lod = meshLods.lods[meshLods.lodCount++];

        lod.meshletOffset = uint32_t(cluster.meshlets.size());
        lod.meshletCount  = uint32_t(appendMeshlets(cluster, vertices, offsetVertices, vertexCount, indices));
        lod.lodError      = lodError * lodScale;

        if (meshLods.lodCount < std::size(meshLods.lods)) {
            size_t nextIndicesTarget = size_t(double(indices.size()) * 0.75);
            size_t nextIndices       = meshopt_simplifyWithAttributes(indices.data(),
                                                                indices.data(),
                                                                indices.size(),
                                                                &vertices->px,
                                                                vertexCount,
                                                                sizeof(VertexNonCompressed),
                                                                &vertices->nx,
                                                                sizeof(VertexNonCompressed),
                                                                normalWeights,
                                                                3,
                                                                nullptr,
                                                                nextIndicesTarget,
                                                                1e-2f,
                                                                0,
                                                                &lodError);
            assert(nextIndices <= indices.size());

            if (nextIndices == indices.size()) {
                break;
            }

            indices.resize(nextIndices);
            meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertexCount);
        }
    }

    for (uint32_t i = 0; i < meshLods.lodCount; ++i) {
        maxMeshlets = std::max(maxMeshlets, meshLods.lods[i].meshletCount);
    }

    return { result, maxMeshlets };
}

void recursive(Cluster& cluster,
               std::unordered_map<const aiMesh*, std::pair<Instance, uint32_t>>& cacheInstances,
               uint32_t& meshletVisibilityOffset,
               const aiScene* sc,
               const aiNode* nd,
               const glm::mat4& parentTransform) {
    unsigned int i;
    unsigned int n = 0;
    aiMatrix4x4 m  = nd->mTransformation;

    m.Transpose();
    glm::mat4 localTransform;
    localTransform[0] = glm::vec4(m.a1, m.a2, m.a3, m.a4);
    localTransform[1] = glm::vec4(m.b1, m.b2, m.b3, m.b4);
    localTransform[2] = glm::vec4(m.c1, m.c2, m.c3, m.c4);
    localTransform[3] = glm::vec4(m.d1, m.d2, m.d3, m.d4);

    const auto worldTransform = parentTransform * localTransform;

    for (; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

        Instance newInstance{};
        uint32_t maxMeshlets{};

        if (auto it = cacheInstances.find(mesh); it != cacheInstances.end()) {
            newInstance = it->second.first;
            maxMeshlets = it->second.second;
        } else {
            auto [i, m]          = loadMesh(cluster, mesh);
            newInstance          = i;
            maxMeshlets          = m;
            cacheInstances[mesh] = { newInstance, maxMeshlets };
        }

        glm::quat rot{};
        glm::vec3 scale{};
        glm::vec3 translate{};
        glm::vec3 skew{};
        glm::vec4 perspective{};
        glm::decompose(worldTransform, scale, rot, translate, skew, perspective);

        newInstance.world            = worldTransform;
        newInstance.inverseWorld     = glm::inverse(newInstance.world);
        newInstance.scale            = std::max(std::max(scale.x, scale.y), scale.z);
        newInstance.visibilityOffset = meshletVisibilityOffset;

        meshletVisibilityOffset += maxMeshlets;

        cluster.instances.push_back(newInstance);
    }

    for (n = 0; n < nd->mNumChildren; ++n) {
        recursive(cluster, cacheInstances, meshletVisibilityOffset, sc, nd->mChildren[n], worldTransform);
    }
}

void Application::createScene() {
    std::unordered_map<const aiMesh*, std::pair<Instance, uint32_t>> cacheInstances;

    auto fileName = "<path>.fbx";

    constexpr auto removePrimitives =
        aiPrimitiveType_POINT | aiPrimitiveType_LINE | aiPrimitiveType_POLYGON | aiPrimitiveType_NGONEncodingFlag;

    constexpr auto flags =
        aiProcess_FindInstances | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenBoundingBoxes;

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, removePrimitives);

    auto scene                       = importer.ReadFile(fileName, flags);
    uint32_t meshletVisibilityOffset = 0;
    recursive(_cluster, cacheInstances, meshletVisibilityOffset, scene, scene->mRootNode, 1.0f);
    importer.FreeScene();

    uploadCluster(_cluster, 0);

    DrawData drawData{};
    drawData.drawCount = glm::uint(_cluster.instanceCount);
    drawData.lodTarget = 0;
    _drawData =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_UNIFORM_BIT, false, "draw_data", &drawData, sizeof(drawData));
}

void Application::uploadCluster(Cluster& cluster, gerium_uint32_t id) {
    std::vector<VertexCompressed> vertices(cluster.vertices.size());
    for (size_t i = 0; i < cluster.vertices.size(); ++i) {
        auto n = glm::vec3(cluster.vertices[i].nx, cluster.vertices[i].ny, cluster.vertices[i].nz) * 127.0f + 127.5f;

        vertices[i].px = meshopt_quantizeHalf(cluster.vertices[i].px);
        vertices[i].py = meshopt_quantizeHalf(cluster.vertices[i].py);
        vertices[i].pz = meshopt_quantizeHalf(cluster.vertices[i].pz);
        vertices[i].nx = uint8_t(n.x);
        vertices[i].ny = uint8_t(n.y);
        vertices[i].nz = uint8_t(n.z);
        vertices[i].tu = meshopt_quantizeHalf(cluster.vertices[i].tu);
        vertices[i].tv = meshopt_quantizeHalf(cluster.vertices[i].tv);
    }

    std::vector<MeshletCompressed> meshlets(cluster.meshlets.size());
    for (size_t i = 0; i < cluster.meshlets.size(); ++i) {
        auto coneAxis8X = (int8_t) meshopt_quantizeSnorm(cluster.meshlets[i].coneAxis[0], 8);
        auto coneAxis8Y = (int8_t) meshopt_quantizeSnorm(cluster.meshlets[i].coneAxis[1], 8);
        auto coneAxis8Z = (int8_t) meshopt_quantizeSnorm(cluster.meshlets[i].coneAxis[2], 8);

        float coneAxis8eX = fabsf(coneAxis8X / 127.f - cluster.meshlets[i].coneAxis[0]);
        float coneAxis8eY = fabsf(coneAxis8Y / 127.f - cluster.meshlets[i].coneAxis[1]);
        float coneAxis8eZ = fabsf(coneAxis8Z / 127.f - cluster.meshlets[i].coneAxis[2]);

        int coneCutoff8 = int(127 * (cluster.meshlets[i].coneCutoff + coneAxis8eX + coneAxis8eY + coneAxis8eZ) + 1);

        meshlets[i].center[0]       = meshopt_quantizeHalf(cluster.meshlets[i].center[0]);
        meshlets[i].center[1]       = meshopt_quantizeHalf(cluster.meshlets[i].center[1]);
        meshlets[i].center[2]       = meshopt_quantizeHalf(cluster.meshlets[i].center[2]);
        meshlets[i].radius          = meshopt_quantizeHalf(cluster.meshlets[i].radius);
        meshlets[i].coneAxis[0]     = coneAxis8X;
        meshlets[i].coneAxis[1]     = coneAxis8Y;
        meshlets[i].coneAxis[2]     = coneAxis8Z;
        meshlets[i].coneCutoff      = (coneCutoff8 > 127) ? 127 : (int8_t) (coneCutoff8);
        meshlets[i].vertexOffset    = cluster.meshlets[i].vertexOffset;
        meshlets[i].primitiveOffset = cluster.meshlets[i].primitiveOffset;
        meshlets[i].vertexCount     = (uint16_t) cluster.meshlets[i].vertexCount;
        meshlets[i].primitiveCount  = (uint16_t) cluster.meshlets[i].primitiveCount;
    }

    std::vector<MeshCompressed> meshes(cluster.meshes.size());
    for (size_t i = 0; i < cluster.meshes.size(); ++i) {
        meshes[i].center[0]  = meshopt_quantizeHalf(cluster.meshes[i].center[0]);
        meshes[i].center[1]  = meshopt_quantizeHalf(cluster.meshes[i].center[1]);
        meshes[i].center[2]  = meshopt_quantizeHalf(cluster.meshes[i].center[2]);
        meshes[i].radius     = meshopt_quantizeHalf(cluster.meshes[i].radius);
        meshes[i].bboxMin[0] = meshopt_quantizeHalf(cluster.meshes[i].bboxMin[0]);
        meshes[i].bboxMin[1] = meshopt_quantizeHalf(cluster.meshes[i].bboxMin[1]);
        meshes[i].bboxMin[2] = meshopt_quantizeHalf(cluster.meshes[i].bboxMin[2]);
        meshes[i].bboxMax[0] = meshopt_quantizeHalf(cluster.meshes[i].bboxMax[0]);
        meshes[i].bboxMax[1] = meshopt_quantizeHalf(cluster.meshes[i].bboxMax[1]);
        meshes[i].bboxMax[2] = meshopt_quantizeHalf(cluster.meshes[i].bboxMax[2]);
        meshes[i].lodCount   = uint8_t(cluster.meshes[i].lodCount);

        for (size_t l = 0; l < cluster.meshes[i].lodCount; ++l) {
            meshes[i].lods[l] = cluster.meshes[i].lods[l];
        }
    }

    auto strId             = '_' + std::to_string(id);
    cluster.verticesBuffer = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                           false,
                                                           "vertices_buffer" + strId,
                                                           vertices.data(),
                                                           sizeof(vertices[0]) * vertices.size());

    cluster.meshletsBuffer = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                           false,
                                                           "meshlets_buffer" + strId,
                                                           meshlets.data(),
                                                           sizeof(meshlets[0]) * meshlets.size());

    cluster.meshesBuffer = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                         false,
                                                         "meshes_buffer" + strId,
                                                         meshes.data(),
                                                         sizeof(meshes[0]) * meshes.size());

    cluster.vertexIndicesBuffer =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                      false,
                                      "vertex_indices_buffer" + strId,
                                      cluster.vertexIndices.data(),
                                      sizeof(cluster.vertexIndices[0]) * cluster.vertexIndices.size());

    cluster.primitiveIndicesBuffer =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                      false,
                                      "primitive_indices_buffer" + strId,
                                      cluster.primitiveIndices.data(),
                                      sizeof(cluster.primitiveIndices[0]) * cluster.primitiveIndices.size());

    cluster.instancesBuffer = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                            false,
                                                            "instances" + strId,
                                                            cluster.instances.data(),
                                                            sizeof(cluster.instances[0]) * cluster.instances.size());

    cluster.instanceCount = gerium_uint32_t(cluster.instances.size());

    cluster.vertices.clear();
    cluster.meshlets.clear();
    cluster.meshes.clear();
    cluster.vertexIndices.clear();
    cluster.primitiveIndices.clear();
    cluster.instances.clear();
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

    _asyncLoader.create(_application, _renderer);
    _resourceManager.create(_asyncLoader, _frameGraph);

    addPass(_presentPass);
    addPass(_gbufferPass);
    addPass(_cullingPass);
    addPass(_indirectPass);
    addPass(_depthPyramidPass);
    addPass(_cullingLatePass);
    addPass(_indirectLatePass);
    addPass(_gbufferLatePass);
    addPass(_debugOcclusionPass);
    addPass(_debugLinePass);

    std::filesystem::path appDir = gerium_file_get_app_dir();
    _resourceManager.loadFrameGraph((appDir / "frame-graphs" / "main.yaml").string());
    _baseTechnique = _resourceManager.loadTechnique((appDir / "techniques" / "base.yaml").string());

    createScene();
    _camera      = Camera(_application, _resourceManager);
    _debugCamera = Camera(_application, _resourceManager);

    for (auto& renderPass : _renderPasses) {
        renderPass->initialize(_frameGraph, _renderer);
    }
}

void Application::uninitialize() {
    if (_renderer) {
        _cluster = {};

        _asyncLoader.destroy();

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
    gerium_frame_graph_enable_node(_frameGraph, "debug_occlusion_pass", _settings.DebugCamera);
    gerium_frame_graph_enable_node(_frameGraph, "debug_line_pass", _settings.DebugCamera);

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

    getDebugCamera()->update();
    getCamera()->update();

    gerium_renderer_render(_renderer, _frameGraph);
    gerium_renderer_present(_renderer);
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
    return app->cppCall([app, elapsedMs]() {
        app->frame(elapsedMs);
    });
}

gerium_bool_t Application::state(gerium_application_t application,
                                 gerium_data_t data,
                                 gerium_application_state_t state) {
    auto app = (Application*) data;
    return app->cppCall([app, state]() {
        app->state(state);
    });
}

gerium_uint32_t Application::prepare(gerium_frame_graph_t frameGraph,
                                     gerium_renderer_t renderer,
                                     gerium_uint32_t maxWorkers,
                                     gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    return renderPass->application()->cppCallInt([renderPass, frameGraph, renderer, maxWorkers]() {
        return renderPass->prepare(frameGraph, renderer, maxWorkers);
    });
}

gerium_bool_t Application::resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer, gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    return renderPass->application()->cppCall([renderPass, frameGraph, renderer]() {
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
    return renderPass->application()->cppCall(
        [renderPass, frameGraph, renderer, commandBuffer, worker, totalWorkers]() {
        renderPass->render(frameGraph, renderer, commandBuffer, worker, totalWorkers);
    });
}
