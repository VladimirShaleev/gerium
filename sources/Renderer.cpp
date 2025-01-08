#include "Renderer.hpp"
#include "FrameGraph.hpp"

namespace gerium {

Renderer::Renderer() noexcept : _shutdownSignal(marl::Event::Mode::Manual), _waitTaskSignal(marl::Event::Mode::Manual) {
}

Renderer::~Renderer() {
}

void Renderer::initialize(gerium_feature_flags_t features, gerium_uint32_t version, bool debug) {
    _logger     = Logger::create("gerium:renderer");
    _loadThread = std::thread([this, scheduler = marl::Scheduler::get()]() {
        scheduler->bind();
        defer(scheduler->unbind());
        loadThread();
    });

    onInitialize(features, version, debug);
}

gerium_feature_flags_t Renderer::getEnabledFeatures() const noexcept {
    return onGetEnabledFeatures();
}

TextureCompressionFlags Renderer::getTextureComperssion() const noexcept {
    return onGetTextureComperssion();
}

bool Renderer::getProfilerEnable() const noexcept {
    return onGetProfilerEnable();
}

void Renderer::setProfilerEnable(bool enable) noexcept {
    onSetProfilerEnable(enable);
}

bool Renderer::isSupportedFormat(gerium_format_t format) noexcept {
    return onIsSupportedFormat(format);
}

void Renderer::getTextureInfo(TextureHandle handle, gerium_texture_info_t& info) noexcept {
    onGetTextureInfo(handle, info);
}

BufferHandle Renderer::createBuffer(const BufferCreation& creation) {
    return onCreateBuffer(creation);
}

TextureHandle Renderer::createTexture(const TextureCreation& creation) {
    if (creation.type == GERIUM_TEXTURE_TYPE_CUBE) {
        if (creation.width != creation.height) {
            error(GERIUM_RESULT_ERROR_INVALID_ARGUMENT);
        }
    }
    return onCreateTexture(creation);
}

TextureHandle Renderer::createTextureView(const TextureViewCreation& creation) {
    return onCreateTextureView(creation);
}

TechniqueHandle Renderer::createTechnique(const FrameGraph& frameGraph,
                                          gerium_utf8_t name,
                                          gerium_uint32_t pipelineCount,
                                          const gerium_pipeline_t* pipelines) {
    return onCreateTechnique(frameGraph, name, pipelineCount, pipelines);
}

DescriptorSetHandle Renderer::createDescriptorSet(bool global) {
    return onCreateDescriptorSet(global);
}

RenderPassHandle Renderer::createRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node) {
    return onCreateRenderPass(frameGraph, node);
}

FramebufferHandle Renderer::createFramebuffer(const FrameGraph& frameGraph,
                                              const FrameGraphNode* node,
                                              gerium_uint32_t textureIndex) {
    return onCreateFramebuffer(frameGraph, node, textureIndex);
}

TextureHandle Renderer::asyncLoadTexture(gerium_utf8_t filename,
                                         gerium_texture_loaded_func_t callback,
                                         gerium_data_t data) {
    if (!File::existsFile(filename)) {
        error(GERIUM_RESULT_ERROR_NOT_FOUND);
    }

    auto file     = File::open(filename, true);
    auto fileSize = file->getSize();
    auto fileData = file->map();

    auto name  = std::filesystem::path(filename).filename().string();
    Task* task = nullptr;

    constexpr char ktx2Identifier[] = { char(-85), 'K', 'T', 'X', ' ', '2', '0', char(-69), '\r', '\n', '\x1A', '\n' };

    if (memcmp(ktx2Identifier, fileData, std::size(ktx2Identifier)) == 0 && fileSize >= KTX_HEADER_SIZE) {
        task = createLoadTaskKtx2(file, name);
    } else {
        task = createLoadTask(file, name);
    }

    task->callback = callback;
    task->userData = data;

    marl::lock lock(_loadRequestsMutex);
    _tasks.push(task);
    _waitTaskSignal.signal();

    return task->texture;
}

void Renderer::asyncUploadTextureData(TextureHandle handle,
                                      gerium_uint8_t mip,
                                      bool generateMips,
                                      gerium_uint32_t textureDataSize,
                                      gerium_cdata_t textureData,
                                      gerium_texture_loaded_func_t callback,
                                      gerium_data_t data) {
    onAsyncUploadTextureData(handle, mip, generateMips, textureDataSize, textureData, callback, data);
}

void Renderer::textureSampler(TextureHandle handle,
                              gerium_filter_t minFilter,
                              gerium_filter_t magFilter,
                              gerium_filter_t mipFilter,
                              gerium_address_mode_t addressModeU,
                              gerium_address_mode_t addressModeV,
                              gerium_address_mode_t addressModeW,
                              gerium_reduction_mode_t reductionMode) {
    onTextureSampler(handle, minFilter, magFilter, mipFilter, addressModeU, addressModeV, addressModeW, reductionMode);
}

BufferHandle Renderer::getBuffer(gerium_utf8_t resource) {
    return onGetBuffer(resource);
}

TextureHandle Renderer::getTexture(gerium_utf8_t resource, bool fromPreviousFrame) {
    return onGetTexture(resource, fromPreviousFrame);
}

void Renderer::destroyBuffer(BufferHandle handle) noexcept {
    onDestroyBuffer(handle);
}

void Renderer::destroyTexture(TextureHandle handle) noexcept {
    onDestroyTexture(handle);
}

void Renderer::destroyTechnique(TechniqueHandle handle) noexcept {
    onDestroyTechnique(handle);
}

void Renderer::destroyDescriptorSet(DescriptorSetHandle handle) noexcept {
    onDestroyDescriptorSet(handle);
}

void Renderer::destroyRenderPass(RenderPassHandle handle) noexcept {
    onDestroyRenderPass(handle);
}

void Renderer::destroyFramebuffer(FramebufferHandle handle) noexcept {
    onDestroyFramebuffer(handle);
}

void Renderer::bind(DescriptorSetHandle handle, gerium_uint16_t binding, BufferHandle buffer) noexcept {
    onBind(handle, binding, buffer);
}

void Renderer::bind(DescriptorSetHandle handle,
                    gerium_uint16_t binding,
                    gerium_uint16_t element,
                    TextureHandle texture) noexcept {
    onBind(handle, binding, element, texture);
}

void Renderer::bind(DescriptorSetHandle handle,
                    gerium_uint16_t binding,
                    gerium_utf8_t resourceInput,
                    bool fromPreviousFrame) noexcept {
    onBind(handle, binding, resourceInput, fromPreviousFrame);
}

gerium_data_t Renderer::mapBuffer(BufferHandle handle, gerium_uint32_t offset, gerium_uint32_t size) noexcept {
    return onMapBuffer(handle, offset, size);
}

void Renderer::unmapBuffer(BufferHandle handle) noexcept {
    onUnmapBuffer(handle);
}

bool Renderer::newFrame() {
    return onNewFrame();
}

void Renderer::render(FrameGraph& frameGraph) {
    onRender(frameGraph);
}

void Renderer::present() {
    onPresent();
}

FfxInterface Renderer::createFfxInterface(gerium_uint32_t maxContexts) {
    return onCreateFfxInterface(maxContexts);
}

void Renderer::destroyFfxInterface(FfxInterface* ffxInterface) noexcept {
    if (ffxInterface && ffxInterface->scratchBuffer) {
        onDestroyFfxInterface(ffxInterface);
    }
}

void Renderer::waitFfxJobs() const noexcept {
    onWaitFfxJobs();
}

FfxResource Renderer::getFfxBuffer(BufferHandle handle) const noexcept {
    return onGetFfxBuffer(handle);
}

FfxResource Renderer::getFfxTexture(TextureHandle handle) const noexcept {
    return onGetFfxTexture(handle);
}

Profiler* Renderer::getProfiler() noexcept {
    return onGetProfiler();
}

void Renderer::getSwapchainSize(gerium_uint16_t& width, gerium_uint16_t& height) const noexcept {
    onGetSwapchainSize(width, height);
}

void Renderer::closeLoadThread() {
    _shutdownSignal.signal();
    _waitTaskSignal.signal();
    _loadThread.join();

    while (!_tasks.empty()) {
        auto task = _tasks.front();
        if (task->ktxTexture) {
            ktxTexture_Destroy(ktxTexture(task->ktxTexture));
        } else if (task->mips.size() && task->mips.back().imageMip == 0) {
            stbi_image_free((void*) task->mips.back().imageData);
        }
        task->file = nullptr;
        delete task;
        _tasks.pop();
    }
}

Renderer::Task* Renderer::createLoadTask(ObjectPtr<File> file, const std::string& name) {
    auto fileSize = file->getSize();
    auto fileData = file->map();

    int comp, width, height;
    stbi_info_from_memory((const stbi_uc*) fileData, (int) fileSize, &width, &height, &comp);

    auto mipLevels = calcMipLevels(width, height);

    TextureCreation tc{};
    tc.setSize((gerium_uint16_t) width, (gerium_uint16_t) height, (gerium_uint16_t) 1)
        .setFlags((gerium_uint16_t) mipLevels, 1, false, false)
        .setFormat(GERIUM_FORMAT_R8G8B8A8_UNORM, GERIUM_TEXTURE_TYPE_2D)
        .setName(name.c_str());

    auto handle = createTexture(tc);

    return new Task{ this, handle, file, fileData, nullptr, true };
}

Renderer::Task* Renderer::createLoadTaskKtx2(ObjectPtr<File> file, const std::string& name) {
    auto fileSize = file->getSize();
    auto fileData = file->map();

    ktxTexture2* texture;
    auto result =
        ktxTexture2_CreateFromMemory((const ktx_uint8_t*) fileData, fileSize, KTX_TEXTURE_CREATE_NO_FLAGS, &texture);
    if (result != KTX_SUCCESS) {
        _logger->print(GERIUM_LOGGER_LEVEL_ERROR, ktxErrorString(result));
        error(GERIUM_RESULT_ERROR_LOAD_TEXTURE);
    }

    auto compressions  = getTextureComperssion();
    auto supportedASTC = (compressions & TextureCompressionFlags::ASTC_LDR) == TextureCompressionFlags::ASTC_LDR;
    auto supportedETC2 = (compressions & TextureCompressionFlags::ETC2) == TextureCompressionFlags::ETC2;
    auto supportedBC   = (compressions & TextureCompressionFlags::BC) == TextureCompressionFlags::BC;

    gerium_format_t format;

    if (ktxTexture2_NeedsTranscoding(texture)) {
        auto colorModel = ktxTexture2_GetColorModel_e(texture);
        if (colorModel == KHR_DF_MODEL_UASTC && supportedASTC) {
            format = GERIUM_FORMAT_ASTC_4x4_UNORM;
        } else if (colorModel == KHR_DF_MODEL_ETC1S && supportedETC2) {
            format = GERIUM_FORMAT_ETC2_R8G8B8_UNORM;
        } else if (supportedASTC) {
            format = GERIUM_FORMAT_ASTC_4x4_UNORM;
        } else if (supportedETC2) {
            format = GERIUM_FORMAT_ETC2_R8G8B8A8_UNORM;
        } else if (supportedBC) {
            format = GERIUM_FORMAT_BC7_UNORM;
        } else {
            _logger->print(GERIUM_LOGGER_LEVEL_ERROR, "not support transcode target format");
            error(GERIUM_RESULT_ERROR_FORMAT_NOT_SUPPORTED);
        }
    } else {
        format = toGeriumFormat(ktxTexture2_GetVkFormat(texture));
        if (!isSupportedFormat(format)) {
            error(GERIUM_RESULT_ERROR_FORMAT_NOT_SUPPORTED);
        }
    }

    gerium_texture_type_t type;
    if (texture->isCubemap) {
        type = GERIUM_TEXTURE_TYPE_CUBE;
    } else {
        switch (texture->numDimensions) {
            case 1:
                type = GERIUM_TEXTURE_TYPE_1D;
                break;
            case 2:
                type = GERIUM_TEXTURE_TYPE_2D;
                break;
            case 3:
                type = GERIUM_TEXTURE_TYPE_3D;
                break;
            default:
                error(GERIUM_RESULT_ERROR_LOAD_TEXTURE);
        }
    }

    auto mips = texture->generateMipmaps ? calcMipLevels(texture->baseWidth, texture->baseHeight) : texture->numLevels;

    TextureCreation tc{};
    tc.setSize((gerium_uint16_t) texture->baseWidth,
               (gerium_uint16_t) texture->baseHeight,
               (gerium_uint16_t) texture->baseDepth)
        .setFlags(mips, 1, false, false)
        .setFormat(format, type)
        .setName(name.c_str());

    auto handle = createTexture(tc);

    return new Task{ this, handle, file, fileData, texture, texture->generateMipmaps };
}

void Renderer::loadThread() noexcept {
    while (!_shutdownSignal.test()) {
        _waitTaskSignal.wait();

        std::vector<Task*> tasks;
        {
            marl::lock lock(_loadRequestsMutex);
            tasks.reserve(_tasks.size());
            while (!_tasks.empty()) {
                tasks.push_back(_tasks.front());
                _tasks.pop();
            }
            _waitTaskSignal.clear();
        }

        for (auto task : tasks) {
            if (task->ktxTexture && task->mips.empty()) {
                auto compressions = getTextureComperssion();
                auto supportedASTC =
                    (compressions & TextureCompressionFlags::ASTC_LDR) == TextureCompressionFlags::ASTC_LDR;
                auto supportedETC2 = (compressions & TextureCompressionFlags::ETC2) == TextureCompressionFlags::ETC2;
                auto supportedBC   = (compressions & TextureCompressionFlags::BC) == TextureCompressionFlags::BC;

                if (ktxTexture2_NeedsTranscoding(task->ktxTexture)) {
                    ktx_texture_transcode_fmt_e tf;
                    auto colorModel = ktxTexture2_GetColorModel_e(task->ktxTexture);
                    if (colorModel == KHR_DF_MODEL_UASTC && supportedASTC) {
                        tf = KTX_TTF_ASTC_4x4_RGBA;
                    } else if (colorModel == KHR_DF_MODEL_ETC1S && supportedETC2) {
                        tf = KTX_TTF_ETC;
                    } else if (supportedASTC) {
                        tf = KTX_TTF_ASTC_4x4_RGBA;
                    } else if (supportedETC2) {
                        tf = KTX_TTF_ETC2_RGBA;
                    } else if (supportedBC) {
                        tf = KTX_TTF_BC7_RGBA;
                    }
                    ktxTexture2_TranscodeBasis(task->ktxTexture, tf, 0);
                }

                ktxTexture_IterateLevels(ktxTexture(task->ktxTexture), loadMips, &task->mips);
            } else if (task->mips.empty()) {
                int widht, height, comp;
                auto imageData = (gerium_cdata_t) stbi_load_from_memory(
                    (const stbi_uc*) task->data, (int) task->file->getSize(), &widht, &height, &comp, 4);
                task->mips.push({ imageData, 0, 0 });
            }

            if (_shutdownSignal.test()) {
                break;
            }

            const auto& taskMip = task->mips.front();

            asyncUploadTextureData(task->texture,
                                   taskMip.imageMip,
                                   task->imageGenerateMips,
                                   taskMip.imageSize,
                                   taskMip.imageData,
                                   [](auto _, auto texture, auto data) {
                auto task = (Task*) data;
                auto mip0 = task->mips.back();
                task->mips.pop();
                if (task->mips.empty()) {
                    if (task->ktxTexture) {
                        ktxTexture_Destroy(ktxTexture(task->ktxTexture));
                    } else {
                        stbi_image_free((void*) mip0.imageData);
                    }
                    task->file = nullptr;
                    if (task->callback) {
                        task->callback(task->renderer, task->texture, task->userData);
                    }
                    delete task;
                } else {
                    marl::lock lock(task->renderer->_loadRequestsMutex);
                    task->renderer->_tasks.push(task);
                    task->renderer->_waitTaskSignal.signal();
                }
            },
                                   task);
        }
    }
}

KTX_error_code Renderer::loadMips(
    int miplevel, int face, int width, int height, int depth, ktx_uint64_t faceLodSize, void* pixels, void* userdata) {
    auto mips = static_cast<std::queue<TaskMip>*>(userdata);
    mips->push({ pixels, (gerium_uint32_t) faceLodSize, (gerium_uint8_t) miplevel });
    return KTX_SUCCESS;
}

} // namespace gerium

using namespace gerium;

gerium_renderer_t gerium_renderer_reference(gerium_renderer_t renderer) {
    assert(renderer);
    renderer->reference();
    return renderer;
}

void gerium_renderer_destroy(gerium_renderer_t renderer) {
    if (renderer) {
        renderer->destroy();
    }
}

gerium_feature_flags_t gerium_renderer_get_enabled_features(gerium_renderer_t renderer) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->getEnabledFeatures();
}

gerium_bool_t gerium_renderer_get_profiler_enable(gerium_renderer_t renderer) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->getProfilerEnable();
}

void gerium_renderer_set_profiler_enable(gerium_renderer_t renderer, gerium_bool_t enable) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->setProfilerEnable(enable);
}

gerium_bool_t gerium_renderer_is_supported_format(gerium_renderer_t renderer, gerium_format_t format) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->isSupportedFormat(format);
}

void gerium_renderer_get_texture_info(gerium_renderer_t renderer,
                                      gerium_texture_h handle,
                                      gerium_texture_info_t* info) {
    assert(renderer);
    assert(info);
    return alias_cast<Renderer*>(renderer)->getTextureInfo({ handle.index }, *info);
}

gerium_result_t gerium_renderer_create_buffer(gerium_renderer_t renderer,
                                              gerium_buffer_usage_flags_t buffer_usage,
                                              gerium_bool_t dynamic,
                                              gerium_utf8_t name,
                                              gerium_cdata_t data,
                                              gerium_uint32_t size,
                                              gerium_buffer_h* handle) {
    assert(renderer);
    GERIUM_ASSERT_ARG(handle);

    BufferCreation bc;
    bc.set(buffer_usage, dynamic ? ResourceUsageType::Dynamic : ResourceUsageType::Immutable, size)
        .setName(name)
        .setInitialData((void*) data);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->createBuffer(bc);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_create_texture(gerium_renderer_t renderer,
                                               const gerium_texture_info_t* info,
                                               gerium_cdata_t data,
                                               gerium_texture_h* handle) {
    assert(renderer);
    GERIUM_ASSERT_ARG(info);
    GERIUM_ASSERT_ARG(handle);

    TextureCreation tc;
    tc.setSize(info->width, info->height, info->depth)
        .setFlags(info->mipmaps, info->layers, false, true)
        .setFormat(info->format, info->type)
        .setData((void*) data)
        .setName(info->name)
        .build();

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->createTexture(tc);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_create_texture_view(gerium_renderer_t renderer,
                                                    gerium_texture_h texture,
                                                    gerium_texture_type_t type,
                                                    gerium_uint16_t mip_base_level,
                                                    gerium_uint16_t mip_level_count,
                                                    gerium_uint16_t layer_base,
                                                    gerium_uint16_t layer_count,
                                                    gerium_utf8_t name,
                                                    gerium_texture_h* handle) {
    assert(renderer);
    GERIUM_ASSERT_ARG(handle);

    TextureViewCreation vc;
    vc.setTexture({ texture.index })
        .setType(type)
        .setMips(mip_base_level, mip_level_count)
        .setArray(layer_base, layer_count)
        .setName(name);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->createTextureView(vc);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_create_technique(gerium_renderer_t renderer,
                                                 gerium_frame_graph_t frame_graph,
                                                 gerium_utf8_t name,
                                                 gerium_uint32_t pipeline_count,
                                                 const gerium_pipeline_t* pipelines,
                                                 gerium_technique_h* handle) {
    assert(renderer);
    GERIUM_ASSERT_ARG(frame_graph);
    GERIUM_ASSERT_ARG(name);
    GERIUM_ASSERT_ARG(pipeline_count > 0);
    GERIUM_ASSERT_ARG(pipelines);
    GERIUM_ASSERT_ARG(handle);
    for (gerium_uint32_t i = 0; i < pipeline_count; ++i) {
        GERIUM_ASSERT_ARG(pipelines[i].rasterization);
        GERIUM_ASSERT_ARG(pipelines[i].depth_stencil);
        GERIUM_ASSERT_ARG(pipelines[i].color_blend);
    }

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->createTechnique(
            *alias_cast<FrameGraph*>(frame_graph), name, pipeline_count, pipelines);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_create_descriptor_set(gerium_renderer_t renderer,
                                                      gerium_bool_t global,
                                                      gerium_descriptor_set_h* handle) {
    assert(renderer);
    GERIUM_ASSERT_ARG(handle);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->createDescriptorSet(global);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_async_load_texture(gerium_renderer_t renderer,
                                                   gerium_utf8_t filename,
                                                   gerium_texture_loaded_func_t callback,
                                                   gerium_data_t data,
                                                   gerium_texture_h* handle) {
    assert(renderer);
    GERIUM_ASSERT_ARG(filename);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->asyncLoadTexture(filename, callback, data);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_async_upload_texture_data(gerium_renderer_t renderer,
                                                          gerium_texture_h handle,
                                                          gerium_cdata_t texture_data,
                                                          gerium_texture_loaded_func_t callback,
                                                          gerium_data_t data) {
    assert(renderer);
    GERIUM_ASSERT_ARG(texture_data);

    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<Renderer*>(renderer)->asyncUploadTextureData(
            { handle.index }, 0, true, 0, texture_data, callback, data);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_texture_sampler(gerium_renderer_t renderer,
                                                gerium_texture_h handle,
                                                gerium_filter_t min_filter,
                                                gerium_filter_t mag_filter,
                                                gerium_filter_t mip_filter,
                                                gerium_address_mode_t address_mode_u,
                                                gerium_address_mode_t address_mode_v,
                                                gerium_address_mode_t address_mode_w,
                                                gerium_reduction_mode_t reduction_mode) {
    assert(renderer);

    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<Renderer*>(renderer)->textureSampler({ handle.index },
                                                        min_filter,
                                                        mag_filter,
                                                        mip_filter,
                                                        address_mode_u,
                                                        address_mode_v,
                                                        address_mode_w,
                                                        reduction_mode);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_get_buffer(gerium_renderer_t renderer,
                                           gerium_utf8_t resource,
                                           gerium_buffer_h* handle) {
    assert(renderer);
    assert(handle);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->getBuffer(resource);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_get_texture(gerium_renderer_t renderer,
                                            gerium_utf8_t resource,
                                            gerium_bool_t from_previous_frame,
                                            gerium_texture_h* handle) {
    assert(renderer);
    assert(handle);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->getTexture(resource, from_previous_frame);
    GERIUM_END_SAFE_BLOCK
}

void gerium_renderer_destroy_buffer(gerium_renderer_t renderer, gerium_buffer_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyBuffer({ handle.index });
}

void gerium_renderer_destroy_texture(gerium_renderer_t renderer, gerium_texture_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyTexture({ handle.index });
}

void gerium_renderer_destroy_technique(gerium_renderer_t renderer, gerium_technique_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyTechnique({ handle.index });
}

void gerium_renderer_destroy_descriptor_set(gerium_renderer_t renderer, gerium_descriptor_set_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyDescriptorSet({ handle.index });
}

void gerium_renderer_bind_buffer(gerium_renderer_t renderer,
                                 gerium_descriptor_set_h handle,
                                 gerium_uint16_t binding,
                                 gerium_buffer_h buffer) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->bind({ handle.index }, binding, BufferHandle{ buffer.index });
}

void gerium_renderer_bind_texture(gerium_renderer_t renderer,
                                  gerium_descriptor_set_h handle,
                                  gerium_uint16_t binding,
                                  gerium_uint16_t element,
                                  gerium_texture_h texture) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->bind({ handle.index }, binding, element, TextureHandle{ texture.index });
}

void gerium_renderer_bind_resource(gerium_renderer_t renderer,
                                   gerium_descriptor_set_h handle,
                                   gerium_uint16_t binding,
                                   gerium_utf8_t resource_input,
                                   gerium_bool_t from_previous_frame) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->bind({ handle.index }, binding, resource_input, from_previous_frame);
}

gerium_data_t gerium_renderer_map_buffer(gerium_renderer_t renderer,
                                         gerium_buffer_h handle,
                                         gerium_uint32_t offset,
                                         gerium_uint32_t size) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->mapBuffer({ handle.index }, offset, size);
}

void gerium_renderer_unmap_buffer(gerium_renderer_t renderer, gerium_buffer_h handle) {
    assert(renderer);
    alias_cast<Renderer*>(renderer)->unmapBuffer({ handle.index });
}

gerium_result_t gerium_renderer_new_frame(gerium_renderer_t renderer) {
    assert(renderer);
    GERIUM_BEGIN_SAFE_BLOCK
        if (!alias_cast<Renderer*>(renderer)->newFrame()) {
            return GERIUM_RESULT_SKIP_FRAME;
        }
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_render(gerium_renderer_t renderer, gerium_frame_graph_t frame_graph) {
    assert(renderer);
    GERIUM_ASSERT_ARG(frame_graph);
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<Renderer*>(renderer)->render(*alias_cast<FrameGraph*>(frame_graph));
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_present(gerium_renderer_t renderer) {
    assert(renderer);
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<Renderer*>(renderer)->present();
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_create_ffx_interface(gerium_renderer_t renderer,
                                                     gerium_uint32_t max_contexts,
                                                     FfxInterface* ffx_interface) {
    assert(renderer);
    GERIUM_BEGIN_SAFE_BLOCK
        *ffx_interface = alias_cast<Renderer*>(renderer)->createFfxInterface(max_contexts);
    GERIUM_END_SAFE_BLOCK
}

void gerium_renderer_destroy_ffx_interface(gerium_renderer_t renderer, FfxInterface* ffx_interface) {
    assert(renderer);
    alias_cast<Renderer*>(renderer)->destroyFfxInterface(ffx_interface);
}

void gerium_renderer_wait_ffx_jobs(gerium_renderer_t renderer) {
    assert(renderer);
    alias_cast<Renderer*>(renderer)->waitFfxJobs();
}

FfxResource gerium_renderer_get_ffx_buffer(gerium_renderer_t renderer, gerium_buffer_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->getFfxBuffer({ handle.index });
}

FfxResource gerium_renderer_get_ffx_texture(gerium_renderer_t renderer, gerium_texture_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->getFfxTexture({ handle.index });
}
