#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "Camera.hpp"
#include "RenderPass.hpp"
#include "ResourceManager.hpp"
#include "Settings.hpp"

struct Cluster {
    gerium_uint32_t instanceCount;
    Buffer vertices;
    Buffer meshlets;
    Buffer vertexIndices;
    Buffer primitiveIndices;
    Buffer shadowVertices;
    Buffer shadowIndices;
    Buffer meshes;
    Buffer instances;
    DescriptorSet descriptorSet;
};

class IndirectPass final : public RenderPass {
public:
    IndirectPass(bool late) : RenderPass(!late ? "indirect_pass" : "indirect_late_pass"), _latePass(late) {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    DescriptorSet _descriptorSet;
    bool _latePass{};
};

class DepthPyramidPass final : public RenderPass {
public:
    DepthPyramidPass() : RenderPass("depth_pyramid_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    void createDepthPyramid(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer);
    static void assignReductionSampler(gerium_renderer_t renderer, gerium_texture_h texture);

    gerium_uint16_t _depthPyramidWidth{};
    gerium_uint16_t _depthPyramidHeight{};
    gerium_uint16_t _depthPyramidMipLevels{};
    Texture _depthPyramid{};
    Texture _depthPyramidMips[16]{};
    Buffer _imageSizes[16]{};
    DescriptorSet _descriptorSets[16]{};
};

class CullingPass final : public RenderPass {
public:
    CullingPass(bool late) : RenderPass(!late ? "culling_pass" : "culling_late_pass"), _latePass(late) {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    DescriptorSet _descriptorSet0{};
    bool _latePass{};
};

class GBufferPass final : public RenderPass {
public:
    GBufferPass(bool late) : RenderPass(!late ? "gbuffer_pass" : "gbuffer_late_pass"), _latePass(late) {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    DescriptorSet _descriptorSet;
    bool _latePass{};
};

class LightPass final : public RenderPass {
public:
    LightPass() : RenderPass("light_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;
};

class PresentPass final : public RenderPass {
public:
    PresentPass() : RenderPass("present_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;
};

class DebugOcclusionPass final : public RenderPass {
public:
    DebugOcclusionPass() : RenderPass("debug_occlusion_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    DescriptorSet _descriptorSet;
};

class DebugLinePass final : public RenderPass {
public:
    DebugLinePass() : RenderPass("debug_line_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    int _maxPoints{};
    DescriptorSet _descriptorSet{};
    Buffer _vertices{};
};

class BSDFPass final : public RenderPass {
public:
    BSDFPass() : RenderPass("bsdf_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

private:
    gerium_uint32_t _frameIndex{};
};

class BGIPass final : public RenderPass {
public:
    BGIPass() : RenderPass("bgi_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    std::array<Texture, 16> _noiseTextures{};
    gerium_uint32_t _frameIndex{};
};

class SkyDomeGenPass final : public RenderPass {
public:
    static constexpr uint16_t kSkySize = 512;

    SkyDomeGenPass() : RenderPass("skydome_gen_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    SkyData _data;

    Technique _technique;
    Texture _skyDome;
    Buffer _skyData;
    DescriptorSet _descriptorSet;
};

class SkyDomePrefilteredPass final : public RenderPass {
public:
    static constexpr uint16_t kSkySize = 512;
    static constexpr uint16_t kMips = 10;

    SkyDomePrefilteredPass() : RenderPass("skydome_prefiltered_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    Technique _technique;
    Texture _skyDomePrefiltered;
    std::array<Texture, kMips> _skyDomeMips;
    std::array<Buffer, kMips> _skyPrefilteredDatas;
    std::array<DescriptorSet, kMips> _descriptorSets;
};

class Application final {
public:
    static constexpr uint32_t kFfxBrixelizerMaxCascades = FFX_BRIXELIZER_MAX_CASCADES;
    static constexpr uint32_t kNumBrixelizerCascades    = kFfxBrixelizerMaxCascades / 3;

    Application();
    ~Application();

    void run(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height);

    ResourceManager& resourceManager() noexcept {
        return _resourceManager;
    }

    Settings& settings() noexcept {
        return _settings;
    }

    bool bindlessSupported() const noexcept {
        return _bindlessSupported;
    }

    bool meshShaderSupported() const noexcept {
        return _meshShaderSupported;
    }

    Camera* getCamera() noexcept {
        return &_camera;
    }

    Camera* getDebugCamera() noexcept {
        return &_debugCamera;
    }

    gerium_uint16_t width() const noexcept {
        return _width;
    }

    gerium_uint16_t height() const noexcept {
        return _height;
    }

    gerium_float32_t invWidth() const noexcept {
        return _invWidth;
    }

    gerium_float32_t invHeight() const noexcept {
        return _invHeight;
    }

    gerium_technique_h getBaseTechnique() const noexcept {
        return _baseTechnique;
    }

    gerium_texture_h brdfLut() const noexcept {
        return _brdfLut;
    }

    gerium_descriptor_set_h texturesSet() const noexcept {
        return _texturesSet;
    }

    Cluster& cluster() noexcept {
        return _cluster;
    }

    gerium_buffer_h drawData() const noexcept {
        return _drawData;
    }

    gerium_texture_h bsdfAtlas() const noexcept {
        return _bsdfAtlas;
    }

    gerium_buffer_h cascadeAABBTrees(gerium_uint32_t index) const noexcept {
        return _cascadeAABBTrees[index];
    }

    gerium_buffer_h cascadeBrickMaps(gerium_uint32_t index) const noexcept {
        return _cascadeBrickMaps[index];
    }

    FfxBrixelizerContext* brixelizerContext() noexcept {
        return _brixelizerContext.get();
    }

    FfxBrixelizerGIContext* brixelizerGIContext() noexcept {
        return _brixelizerGIContext.get();
    }

    FfxBrixelizerBakedUpdateDescription* brixelizerBakedUpdateDesc() noexcept {
        return _brixelizerBakedUpdateDesc.get();
    }

    FfxBrixelizerGIDispatchDescription* brixelizerGIDispatch() noexcept {
        return _brixelizerGIDispatch.get();
    }

private:
    void addPass(RenderPass& renderPass);
    void createBrixelizerContext();
    void createScene();
    Cluster loadCluster(std::string_view name);

    void initialize();
    void uninitialize();

    void pollInput(gerium_uint64_t elapsedMs);
    void frame(gerium_uint64_t elapsedMs);
    void state(gerium_application_state_t state);

    static gerium_bool_t frame(gerium_application_t application, gerium_data_t data, gerium_uint64_t elapsedMs);
    static gerium_bool_t state(gerium_application_t application, gerium_data_t data, gerium_application_state_t state);

    static gerium_uint32_t prepare(gerium_frame_graph_t frameGraph,
                                   gerium_renderer_t renderer,
                                   gerium_uint32_t maxWorkers,
                                   gerium_data_t data);
    static gerium_bool_t resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer, gerium_data_t data);
    static gerium_bool_t render(gerium_frame_graph_t frameGraph,
                                gerium_renderer_t renderer,
                                gerium_command_buffer_t commandBuffer,
                                gerium_uint32_t worker,
                                gerium_uint32_t totalWorkers,
                                gerium_data_t data);

    // We wrap the call into a C++ member function so that we can propagate the exception,
    // if it occurs, back to the C++ code and rethrow it after the C API callback has done
    template <typename Func>
    std::conditional_t<std::is_same_v<std::invoke_result_t<Func>, void>, bool, std::invoke_result_t<Func>>
    cppCallWrap(Func&& func) noexcept {
        using ReturnType = std::invoke_result_t<Func>;

        constexpr auto isVoidReturn = std::is_same_v<ReturnType, void>;
        try {
            if constexpr (isVoidReturn) {
                func();
                return true;
            } else {
                return func();
            }
        } catch (...) {
            uninitialize();
            _error = std::current_exception();
            if constexpr (isVoidReturn) {
                return false;
            } else {
                return ReturnType();
            }
        }
    }

    std::exception_ptr _error{};
    gerium_logger_t _logger{};
    gerium_application_t _application{};
    gerium_renderer_t _renderer{};
    gerium_profiler_t _profiler{};
    gerium_frame_graph_t _frameGraph{};
    bool _bindlessSupported{};
    bool _meshShaderSupported{};

    Settings _settings{};
    DepthPyramidPass _depthPyramidPass{};
    CullingPass _cullingPass{ false };
    CullingPass _cullingLatePass{ true };
    GBufferPass _gbufferPass{ false };
    GBufferPass _gbufferLatePass{ true };
    PresentPass _presentPass{};
    IndirectPass _indirectPass{ false };
    IndirectPass _indirectLatePass{ true };
    DebugOcclusionPass _debugOcclusionPass{};
    LightPass _lightPass{};
    DebugLinePass _debugLinePass{};
    BSDFPass _bsdfPass{};
    BGIPass _bgiPass{};
    SkyDomeGenPass _skyDomeGen{};
    SkyDomePrefilteredPass _skyDomePrefilteredPass{};
    std::vector<RenderPass*> _renderPasses{};

    AsyncLoader _asyncLoader{};
    ResourceManager _resourceManager{};
    Camera _camera{};
    Camera _debugCamera{};

    gerium_uint16_t _prevWidth{};
    gerium_uint16_t _prevHeight{};
    gerium_uint16_t _width{};
    gerium_uint16_t _height{};
    gerium_float32_t _invWidth{};
    gerium_float32_t _invHeight{};

    Technique _baseTechnique{};

    Texture _brdfLut{};
    Texture _emptyTexture{};
    std::vector<Texture> _textures{};
    DescriptorSet _texturesSet{};
    Cluster _cluster{};
    Buffer _drawData{};

    std::unique_ptr<FfxBrixelizerContextDescription> _brixelizerParams{};
    std::unique_ptr<FfxBrixelizerContext> _brixelizerContext{};
    std::unique_ptr<FfxBrixelizerGIContext> _brixelizerGIContext{};
    std::unique_ptr<FfxBrixelizerBakedUpdateDescription> _brixelizerBakedUpdateDesc{};
    std::unique_ptr<FfxBrixelizerGIDispatchDescription> _brixelizerGIDispatch{};
    std::array<uint32_t, 2> _brixelizerBuffers{};
    std::vector<FfxBrixelizerInstanceID> _brixelizerInstances{};
    Texture _bsdfAtlas{};
    Buffer _cascadeAABBTrees[FFX_BRIXELIZER_MAX_CASCADES]{};
    Buffer _cascadeBrickMaps[FFX_BRIXELIZER_MAX_CASCADES]{};
};

#endif
