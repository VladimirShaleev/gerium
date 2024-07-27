/*
 * The current organization of Vulkan resources is mainly taken from this project:
 *   https://github.com/PacktPublishing/Mastering-Graphics-Programming-with-Vulkan
 */

#ifndef GERIUM_WINDOWS_VULKAN_RESOURCES_HPP
#define GERIUM_WINDOWS_VULKAN_RESOURCES_HPP

#include "../Gerium.hpp"
#include "../Handles.hpp"
#include "Enums.hpp"

namespace gerium::vulkan {

// clang-format off

constexpr uint32_t kMaxFrames               = 2; // TODO: duplicated in Device
constexpr uint8_t  kMaxImageOutputs         = 8;
constexpr uint8_t  kMaxDescriptorSetLayouts = 8;
constexpr uint8_t  kMaxDescriptorsPerSet    = 16;
constexpr uint8_t  kMaxVertexBindings       = 16;
constexpr uint8_t  kMaxVertexAttributes     = 16;
constexpr uint8_t  kMaxShaderStages         = 5;
constexpr uint8_t  kMaxMaterialPasses       = 20;
constexpr uint32_t kGlobalPoolElements      = 1024;
constexpr uint32_t kDescriptorSetsPoolSize  = 1024;

struct SamplerHandle : Handle {};
struct DescriptorSetLayoutHandle : Handle {};
struct ProgramHandle : Handle {};
struct PipelineHandle : Handle {};

using BufferPool              = ResourcePool<struct Buffer, BufferHandle>;
using TexturePool             = ResourcePool<struct Texture, TextureHandle>;
using SamplerPool             = ResourcePool<struct Sampler, SamplerHandle>;
using RenderPassPool          = ResourcePool<struct RenderPass, RenderPassHandle>;
using DescriptorSetPool       = ResourcePool<struct DescriptorSet, DescriptorSetHandle>;
using DescriptorSetLayoutPool = ResourcePool<struct DescriptorSetLayout, DescriptorSetLayoutHandle>;
using ProgramPool             = ResourcePool<struct Program, ProgramHandle>;
using PipelinePool            = ResourcePool<struct Pipeline, PipelineHandle>;
using FramebufferPool         = ResourcePool<struct Framebuffer, FramebufferHandle>;
using MaterialPool            = ResourcePool<struct Material, MaterialHandle>;

struct SamplerCreation {
    VkFilter            minFilter = VK_FILTER_NEAREST;
    VkFilter            magFilter = VK_FILTER_NEAREST;
    VkSamplerMipmapMode mipFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;

    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    const char* name = nullptr;

    SamplerCreation& setMinMagMip(VkFilter min, VkFilter mag, VkSamplerMipmapMode mip) {
        minFilter = min;
        magFilter = mag;
        mipFilter = mip;
        return *this;
    }

    SamplerCreation& setAddressModeU(VkSamplerAddressMode u) {
        addressModeU = u;
        return *this;
    }

    SamplerCreation& setAddressModeUv(VkSamplerAddressMode u, VkSamplerAddressMode v) {
        addressModeU = u;
        addressModeV = v;
        return *this;
    }

    SamplerCreation& setAddressModeUvw(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w) {
        addressModeU = u;
        addressModeV = v;
        addressModeW = w;
        return *this;
    }

    SamplerCreation& setName(const char* name) {
        this->name = name;
        return *this;
    }
};

struct RenderPassOutput {
    uint32_t                numColorFormats;
    VkFormat                colorFormats[kMaxImageOutputs];
    VkImageLayout           colorFinalLayouts[kMaxImageOutputs];
    gerium_render_pass_op_t colorOperations[kMaxImageOutputs];

    VkFormat                depthStencilFormat;
    VkImageLayout           depthStencilFinalLayout;
    gerium_render_pass_op_t depthOperation;
    gerium_render_pass_op_t stencilOperation;

    RenderPassOutput& color(VkFormat format, VkImageLayout layout, gerium_render_pass_op_t loadOp) {
        assert(numColorFormats < kMaxImageOutputs);
        colorFormats[numColorFormats]      = format;
        colorFinalLayouts[numColorFormats] = layout;
        colorOperations[numColorFormats]   = loadOp;
        ++numColorFormats;
        return *this;
    }

    RenderPassOutput& depth(VkFormat format, VkImageLayout layout) {
        depthStencilFormat      = format;
        depthStencilFinalLayout = layout;
        return *this;
    }

    RenderPassOutput& setDepthStencilOperations(gerium_render_pass_op_t depth, gerium_render_pass_op_t stencil) {
        depthOperation   = depth;
        stencilOperation = stencil;
        return *this;
    }
};

struct RenderPassCreation {
    RenderPassOutput output = {};
    const char*      name   = nullptr;

    RenderPassCreation& setName(const char* name) {
        this->name = name;
        return *this;
    }
};

struct DescriptorSetLayoutData {
    uint32_t setNumber;
    gerium_uint64_t hash;
    VkDescriptorSetLayoutCreateInfo createInfo;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
};

struct DescriptorSetLayoutCreation {
    const DescriptorSetLayoutData* setLayout;

    const char* name = nullptr;

    DescriptorSetLayoutCreation& setName(const char* name) {
        this->name = name;
        return *this;
    }
};

struct StencilOperationState {
    VkStencilOp fail        = VK_STENCIL_OP_KEEP;
    VkStencilOp pass        = VK_STENCIL_OP_KEEP;
    VkStencilOp depthFail   = VK_STENCIL_OP_KEEP;
    VkCompareOp compare     = VK_COMPARE_OP_ALWAYS;
    uint32_t    compareMask = 0xff;
    uint32_t    writeMask   = 0xff;
    uint32_t    reference   = 0xff;
};

struct DepthStencilCreation {
    StencilOperationState front;
    StencilOperationState back;
    VkCompareOp           depthComparison = VK_COMPARE_OP_ALWAYS;

    uint8_t depthEnable      : 1;
    uint8_t depthWriteEnable : 1;
    uint8_t stencilEnable    : 1;
    uint8_t pad              : 5;

    DepthStencilCreation() : depthEnable(0), depthWriteEnable(0), stencilEnable(0) {
    }

    DepthStencilCreation& setDepth(bool write, VkCompareOp comparisonTest) {
        depthWriteEnable = write;
        depthComparison  = comparisonTest;
        depthEnable      = 1;
        return *this;
    }
};

struct BlendStateCreation {
    gerium_color_component_flags_t writeMasks[kMaxImageOutputs]{};
    gerium_color_blend_attachment_state_t blendStates[kMaxImageOutputs]{};
    uint32_t activeStates = 0;

    BlendStateCreation& reset() {
        activeStates = 0;
        return *this;
    }

    BlendStateCreation& addBlendState(gerium_color_component_flags_t writeMask, const gerium_color_blend_attachment_state_t& state) {
        writeMasks[activeStates] = writeMask;
        blendStates[activeStates++] = state;
        return *this;
    }
};

struct VertexInputCreation {
    gerium_uint32_t numVertexBindings   = 0;
    gerium_uint32_t numVertexAttributes = 0;

    gerium_vertex_binding_t   vertexBindings[kMaxVertexBindings]{};
    gerium_vertex_attribute_t vertexAttributes[kMaxVertexAttributes]{};

    VertexInputCreation& reset() {
        numVertexBindings = 0;
        numVertexAttributes = 0;
        return *this;
    }

    VertexInputCreation& addVertexBinding(const gerium_vertex_binding_t& binding) {
        vertexBindings[numVertexBindings++] = binding;
        return *this;
    }

    VertexInputCreation& addVertexAttribute(const gerium_vertex_attribute_t& attribute) {
        vertexAttributes[numVertexAttributes++] = attribute;
        return *this;
    }
};

struct ProgramCreation {
    uint32_t stagesCount = 0;
    gerium_shader_t stages[kMaxShaderStages]{};

    gerium_utf8_t name = nullptr;

    // Building helpers
    ProgramCreation& reset() {
        stagesCount = 0;
        name = nullptr;
        return *this;
    }

    ProgramCreation& addStage(const gerium_shader_t& stage) {
        stages[stagesCount++] = stage;
        return *this;
    }

    ProgramCreation& setName(const gerium_utf8_t name) {
        this->name = name;
        return *this;
    }
};

struct PipelineCreation {
    const gerium_rasterization_state_t* rasterization = {};
    const gerium_depth_stencil_state_t* depthStencil = {};
    const gerium_color_blend_state_t*   colorBlend = {};
    BlendStateCreation    blendState = {};
    VertexInputCreation   vertexInput = {};
    ProgramCreation       program = {};

    RenderPassOutput          renderPass = {};
    //DescriptorSetLayoutHandle descriptorSetLayout[kMaxDescriptorSetLayouts];
    //const ViewportState*      viewport = nullptr;

    //uint32_t numActiveLayouts = 0;

    const char* name = nullptr;

    //PipelineCreation& addDescriptorSetLayout(DescriptorSetLayoutHandle handle) {
    //    descriptorSetLayout[numActiveLayouts++] = handle;
    //    return *this;
    //}

    RenderPassOutput& renderPassOutput() {
        return renderPass;
    }
};

struct FramebufferCreation {
    RenderPassHandle renderPass;

    uint16_t      numRenderTargets                 = 0;
    TextureHandle outputTextures[kMaxImageOutputs] = {};
    TextureHandle depthStencilTexture              = Undefined;

    uint16_t width  = 0;
    uint16_t height = 0;
    float    scaleX = 1.0f;
    float    scaleY = 1.0f;
    uint8_t  resize = 1;

    const char* name = nullptr;

    FramebufferCreation& addRenderTexture(TextureHandle texture) {
        outputTextures[numRenderTargets++] = texture;
        return *this;
    }

    FramebufferCreation& setDepthStencilTexture(TextureHandle texture) {
        depthStencilTexture = texture;
        return *this;
    }

    FramebufferCreation& setScaling(float scaleX, float scaleY, uint8_t resize) {
        this->scaleX = scaleX;
        this->scaleY = scaleY;
        this->resize = resize;
        return *this;
    }

    FramebufferCreation& setName(const char* name) {
        this->name = name;
        return *this;
    }
};

struct DescriptorSetCreation {
    Handle      resources[kMaxDescriptorsPerSet] {Undefined};
    SamplerHandle samplers[kMaxDescriptorsPerSet] {Undefined};
    uint16_t      bindings[kMaxDescriptorsPerSet] {};

    DescriptorSetLayoutHandle layout       = Undefined;
    uint32_t                  numResources = 0;

    const char* name = nullptr;

    DescriptorSetCreation& reset() {
        numResources = 0;
        return *this;
    }

    DescriptorSetCreation& setLayout(DescriptorSetLayoutHandle layout) {
        this->layout = layout;
        return *this;
    }

    DescriptorSetCreation& texture(TextureHandle texture, uint16_t binding) {
        resources[numResources]  = texture;
        //samplers[numResources]   = SamplerPool::Undefined;
        bindings[numResources++] = binding;
        return *this;
    }

    DescriptorSetCreation& buffer(BufferHandle buffer, uint16_t binding) {
        resources[numResources]  = buffer;
        //samplers[numResources]   = SamplerPool::Undefined;
        bindings[numResources++] = binding;
        return *this;
    }

    // DescriptorSetCreation& textureSampler(TextureHandle texture, SamplerHandle sampler, uint16_t binding) {
    //     resources[numResources]  = texture.index;
    //     //samplers[numResources]   = sampler;
    //     bindings[numResources++] = binding;
    //     return *this;
    // }

    DescriptorSetCreation& setName(const char* name) {
        this->name = name;
        return *this;
    }
};

/*struct DescriptorBinding {
    VkDescriptorType type;
    uint16_t         index;
    uint16_t         count;
    // uint16_t         set;

    const char* name;
};*/

struct Buffer {
    VkBuffer           vkBuffer;
    VmaAllocation      vmaAllocation;
    VkDeviceMemory     vkDeviceMemory;
    VkBufferUsageFlags vkUsageFlags;
    ResourceUsageType  usage;
    gerium_uint32_t    size;
    gerium_uint32_t    globalOffset;
    void*              mappedData;
    gerium_utf8_t      name;
    BufferHandle       parent;
};

struct Texture {
    VkImage               vkImage;
    VkImageView           vkImageView;
    VkFormat              vkFormat;
    VkImageLayout         vkImageLayout;
    VmaAllocation         vmaAllocation;
    uint16_t              width;
    uint16_t              height;
    uint16_t              depth;
    uint8_t               mipmaps;
    TextureFlags          flags;
    gerium_texture_type_t type;
    gerium_utf8_t         name;

    SamplerHandle sampler;
};

struct Sampler {
    VkSampler vkSampler;

    VkFilter            minFilter;
    VkFilter            magFilter;
    VkSamplerMipmapMode mipFilter;

    VkSamplerAddressMode addressModeU;
    VkSamplerAddressMode addressModeV;
    VkSamplerAddressMode addressModeW;

    gerium_utf8_t name;
};

struct RenderPass {
    VkRenderPass     vkRenderPass;
    RenderPassOutput output;
    gerium_utf8_t    name;
};

struct DescriptorSet {
    struct Descriptors {
        VkDescriptorSet vkDescriptorSet[kMaxFrames];
        gerium_uint8_t  current;
    };

    absl::flat_hash_map<gerium_uint64_t, Descriptors> descriptors;

    // DescriptorSetLayoutHandle layout;

    //gerium_uint8_t            numResources;

    // Handle        resources[kMaxDescriptorsPerSet];
    // SamplerHandle samplers[kMaxDescriptorsPerSet];
    Handle bindings[kMaxDescriptorsPerSet];
    gerium_utf8_t resources[kMaxDescriptorsPerSet];
    gerium_uint8_t dirty;
    gerium_uint8_t hasResources;
};

struct DescriptorSetLayout {
    VkDescriptorSetLayout vkDescriptorSetLayout;

    DescriptorSetLayoutData data;
};

struct Program {
    VkPipelineShaderStageCreateInfo shaderStageInfo[kMaxShaderStages];

    const char* name;

    uint32_t activeShaders;
    bool     graphicsPipeline;

    std::vector<uint32_t> spirv[kMaxShaderStages];

    absl::flat_hash_map<uint32_t, DescriptorSetLayoutData> descriptorSets;
};

struct Pipeline {
    VkPipeline       vkPipeline;
    VkPipelineLayout vkPipelineLayout;

    VkPipelineBindPoint vkBindPoint;

    // ProgramHandle program;
    RenderPassHandle renderPass;

    // const DescriptorSetLayout* descriptorSetLayout[k_max_descriptor_set_layouts];
    DescriptorSetLayoutHandle descriptorSetLayoutHandles[kMaxDescriptorSetLayouts];
    uint32_t                  numActiveLayouts;

    // DepthStencilCreation  depthStencil;
    // BlendStateCreation    blendState;
    // RasterizationCreation rasterization;

    PipelineHandle handle;
    bool           graphicsPipeline;
};

struct Framebuffer {
    VkFramebuffer    vkFramebuffer;
    RenderPassHandle renderPass;
    uint16_t         width;
    uint16_t         height;
    float            scaleX;
    float            scaleY;

    gerium_uint32_t numColorAttachments;
    TextureHandle   colorAttachments[kMaxImageOutputs];
    TextureHandle   depthStencilAttachment;

    uint8_t resize;

    gerium_utf8_t name;
};

struct MaterialPass {
    gerium_utf8_t  render_pass;
    PipelineHandle pipeline;
};

struct Material {
    gerium_utf8_t   name;
    gerium_uint32_t passCount;
    MaterialPass    passes[kMaxMaterialPasses];
};

// clang-format on

} // namespace gerium::vulkan

#endif
