#ifndef GERIUM_VULKAN_RESOURCES_HPP
#define GERIUM_VULKAN_RESOURCES_HPP

#include "../Gerium.hpp"
#include "../Handles.hpp"
#include "Enums.hpp"

namespace gerium::vulkan {

// clang-format off

constexpr gerium_uint32_t kMaxFrames               = 2;
constexpr gerium_uint8_t  kMaxImageOutputs         = 8;
constexpr gerium_uint8_t  kMaxDescriptorSetLayouts = 4;
constexpr gerium_uint8_t  kMaxDescriptorsPerSet    = 16;
constexpr gerium_uint8_t  kMaxVertexBindings       = 16;
constexpr gerium_uint8_t  kMaxVertexAttributes     = 16;
constexpr gerium_uint8_t  kMaxShaderStages         = 5;
constexpr gerium_uint8_t  kMaxTechniquePasses      = 20;

struct SamplerHandle             : Handle {};
struct DescriptorSetLayoutHandle : Handle {};
struct ProgramHandle             : Handle {};
struct PipelineHandle            : Handle {};

using BufferPool              = ResourcePool<struct Buffer,              BufferHandle>;
using TexturePool             = ResourcePool<struct Texture,             TextureHandle>;
using SamplerPool             = ResourcePool<struct Sampler,             SamplerHandle>;
using RenderPassPool          = ResourcePool<struct RenderPass,          RenderPassHandle>;
using DescriptorSetPool       = ResourcePool<struct DescriptorSet,       DescriptorSetHandle>;
using DescriptorSetLayoutPool = ResourcePool<struct DescriptorSetLayout, DescriptorSetLayoutHandle>;
using ProgramPool             = ResourcePool<struct Program,             ProgramHandle>;
using PipelinePool            = ResourcePool<struct Pipeline,            PipelineHandle>;
using FramebufferPool         = ResourcePool<struct Framebuffer,         FramebufferHandle>;
using TechniquePool           = ResourcePool<struct Technique,           TechniqueHandle>;

struct SamplerCreation {
    VkFilter               minFilter     = VK_FILTER_NEAREST;
    VkFilter               magFilter     = VK_FILTER_NEAREST;
    VkSamplerMipmapMode    mipFilter     = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    VkSamplerAddressMode   addressModeU  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode   addressModeV  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode   addressModeW  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerReductionMode reductionMode = VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;

    gerium_utf8_t name = nullptr;

    SamplerCreation& setMinMagMip(VkFilter min, VkFilter mag, VkSamplerMipmapMode mip) noexcept {
        minFilter = min;
        magFilter = mag;
        mipFilter = mip;
        return *this;
    }

    SamplerCreation& setAddressModeU(VkSamplerAddressMode u) noexcept {
        addressModeU = u;
        return *this;
    }

    SamplerCreation& setAddressModeUv(VkSamplerAddressMode u, VkSamplerAddressMode v) noexcept {
        addressModeU = u;
        addressModeV = v;
        return *this;
    }

    SamplerCreation& setAddressModeUvw(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w) noexcept {
        addressModeU = u;
        addressModeV = v;
        addressModeW = w;
        return *this;
    }

    SamplerCreation& setReductionMode(VkSamplerReductionMode reduction) noexcept {
        reductionMode = reduction;
        return *this;
    }

    SamplerCreation& setName(gerium_utf8_t name) noexcept {
        this->name = name;
        return *this;
    }
};

struct RenderPassOutput {
    gerium_uint32_t numColorFormats = 0;
    VkFormat        colorFormats[kMaxImageOutputs];
    VkImageLayout   colorFinalLayouts[kMaxImageOutputs];
    RenderPassOp    colorOperations[kMaxImageOutputs];

    VkFormat      depthStencilFormat      = {};
    VkImageLayout depthStencilFinalLayout = {};
    RenderPassOp  depthOperation          = {};
    RenderPassOp  stencilOperation        = {};

    RenderPassOutput& color(VkFormat format, VkImageLayout layout, RenderPassOp loadOp) noexcept {
        assert(numColorFormats < kMaxImageOutputs);
        colorFormats[numColorFormats]      = format;
        colorFinalLayouts[numColorFormats] = layout;
        colorOperations[numColorFormats]   = loadOp;
        ++numColorFormats;
        return *this;
    }

    RenderPassOutput& depth(VkFormat format, VkImageLayout layout) noexcept {
        depthStencilFormat      = format;
        depthStencilFinalLayout = layout;
        return *this;
    }

    RenderPassOutput& setDepthStencilOperations(RenderPassOp depth, RenderPassOp stencil) noexcept {
        depthOperation   = depth;
        stencilOperation = stencil;
        return *this;
    }
};

struct RenderPassCreation {
    RenderPassOutput output = {};
    gerium_utf8_t    name   = nullptr;

    RenderPassCreation& setName(gerium_utf8_t name) noexcept {
        this->name = name;
        return *this;
    }
};

struct DescriptorSetLayoutData {
    gerium_uint32_t                             setNumber;
    gerium_uint64_t                             hash;
    VkDescriptorSetLayoutCreateInfo             createInfo;
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindlessInfo;
    std::vector<VkDescriptorSetLayoutBinding>   bindings;
    std::vector<VkDescriptorBindingFlags>       bindlessFlags;
    std::set<gerium_uint32_t>                   default3DTextures;
};

struct DescriptorSetLayoutCreation {
    const DescriptorSetLayoutData* setLayout = nullptr;

    gerium_utf8_t name = nullptr;

    DescriptorSetLayoutCreation& setName(gerium_utf8_t name) noexcept {
        this->name = name;
        return *this;
    }
};

struct StencilOperationState {
    VkStencilOp     fail        = VK_STENCIL_OP_KEEP;
    VkStencilOp     pass        = VK_STENCIL_OP_KEEP;
    VkStencilOp     depthFail   = VK_STENCIL_OP_KEEP;
    VkCompareOp     compare     = VK_COMPARE_OP_ALWAYS;
    gerium_uint32_t compareMask = 0xff;
    gerium_uint32_t writeMask   = 0xff;
    gerium_uint32_t reference   = 0xff;
};

struct DepthStencilCreation {
    StencilOperationState front;
    StencilOperationState back;
    VkCompareOp           depthComparison = VK_COMPARE_OP_ALWAYS;

    gerium_uint8_t depthEnable      : 1;
    gerium_uint8_t depthWriteEnable : 1;
    gerium_uint8_t stencilEnable    : 1;
    gerium_uint8_t pad              : 5;

    DepthStencilCreation() noexcept : depthEnable(0), depthWriteEnable(0), stencilEnable(0) {
    }

    DepthStencilCreation& setDepth(bool write, VkCompareOp comparisonTest) noexcept {
        depthWriteEnable = write;
        depthComparison  = comparisonTest;
        depthEnable      = 1;
        return *this;
    }
};

struct BlendStateCreation {
    gerium_color_component_flags_t writeMasks[kMaxImageOutputs]{};
    gerium_color_blend_attachment_state_t blendStates[kMaxImageOutputs]{};
    gerium_uint32_t activeStates = 0;

    BlendStateCreation& addBlendState(gerium_color_component_flags_t writeMask, const gerium_color_blend_attachment_state_t& state) noexcept {
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

    VertexInputCreation& addVertexBinding(const gerium_vertex_binding_t& binding) noexcept {
        vertexBindings[numVertexBindings++] = binding;
        return *this;
    }

    VertexInputCreation& addVertexAttribute(const gerium_vertex_attribute_t& attribute) noexcept {
        vertexAttributes[numVertexAttributes++] = attribute;
        return *this;
    }
};

struct ProgramCreation {
    gerium_uint32_t stagesCount = 0;
    gerium_shader_t stages[kMaxShaderStages]{};

    gerium_utf8_t name = nullptr;

    ProgramCreation& addStage(const gerium_shader_t& stage) noexcept {
        stages[stagesCount++] = stage;
        return *this;
    }

    ProgramCreation& setName(const gerium_utf8_t name) noexcept {
        this->name = name;
        return *this;
    }
};

struct PipelineCreation {
    const gerium_rasterization_state_t* rasterization = {};
    const gerium_depth_stencil_state_t* depthStencil  = {};
    const gerium_color_blend_state_t*   colorBlend    = {};

    BlendStateCreation  blendState  = {};
    VertexInputCreation vertexInput = {};
    ProgramCreation     program     = {};

    RenderPassOutput renderPass = {};
    
    gerium_utf8_t name = nullptr;
};

struct FramebufferCreation {
    RenderPassHandle renderPass = Undefined;

    gerium_uint16_t numRenderTargets                 = 0;
    TextureHandle   outputTextures[kMaxImageOutputs] = {};
    TextureHandle   depthStencilTexture              = Undefined;

    gerium_uint16_t  width  = 0;
    gerium_uint16_t  height = 0;
    gerium_uint16_t  layers = 1;
    gerium_float32_t scaleX = 1.0f;
    gerium_float32_t scaleY = 1.0f;
    gerium_uint8_t   resize = 1;

    gerium_utf8_t name = nullptr;

    FramebufferCreation& addRenderTexture(TextureHandle texture) noexcept {
        outputTextures[numRenderTargets++] = texture;
        return *this;
    }

    FramebufferCreation& setDepthStencilTexture(TextureHandle texture) noexcept {
        depthStencilTexture = texture;
        return *this;
    }

    FramebufferCreation& setScaling(gerium_float32_t scaleX, gerium_float32_t scaleY, gerium_uint8_t resize) noexcept {
        this->scaleX = scaleX;
        this->scaleY = scaleY;
        this->resize = resize;
        return *this;
    }

    FramebufferCreation& setName(gerium_utf8_t name) noexcept {
        this->name = name;
        return *this;
    }
};

struct DescriptorSetCreation {
    bool global = false;

    gerium_utf8_t name = nullptr;

    DescriptorSetCreation& setGlobal(bool isGlobal) noexcept {
        global = isGlobal;
        return *this;
    }

    DescriptorSetCreation& setName(gerium_utf8_t name) noexcept {
        this->name = name;
        return *this;
    }
};

struct Buffer {
    VkBuffer           vkBuffer;
    VmaAllocation      vmaAllocation;
    VkDeviceMemory     vkDeviceMemory;
    VkBufferUsageFlags vkUsageFlags;
    ResourceUsageType  usage;
    ResourceState      state;
    gerium_uint32_t    size;
    gerium_uint32_t    globalOffset;
    gerium_data_t      mappedData;
    gerium_uint32_t    mappedOffset;
    gerium_uint32_t    mappedSize;
    gerium_utf8_t      name;
    BufferHandle       parent;
};

struct Texture {
    VkImage               vkImage;
    VkImageView           vkImageView;
    VkFormat              vkFormat;
    VmaAllocation         vmaAllocation;
    gerium_uint32_t       size;
    gerium_uint16_t       width;
    gerium_uint16_t       height;
    gerium_uint16_t       depth;
    gerium_uint8_t        mipBase;
    gerium_uint8_t        mipLevels;
    gerium_uint8_t        layers;
    gerium_uint8_t        loadedMips;
    TextureFlags          flags;
    gerium_texture_type_t type;
    gerium_utf8_t         name;
    TextureHandle         parentTexture;
    SamplerHandle         sampler;
    ResourceState         states[16];
};

struct Sampler {
    VkSampler       vkSampler;
    gerium_uint64_t hash;
    gerium_utf8_t   name;
};

struct RenderPass {
    VkRenderPass     vkRenderPass;
    RenderPassOutput output;
    gerium_uint64_t  hash;
    gerium_utf8_t    name;
};

struct DescriptorSet {
    struct Binding {
        gerium_uint16_t binding;
        gerium_uint16_t element;
        gerium_utf8_t   resource;
        bool            previousFrame;
        Handle          handle;
    };
    VkDescriptorSet                               vkDescriptorSet;
    DescriptorSetLayoutHandle                     layout;
    absl::flat_hash_map<gerium_uint32_t, Binding> bindings;
    gerium_uint8_t                                absoluteFrame;
    gerium_uint8_t                                changed;
    gerium_uint8_t                                global;
};

struct DescriptorSetLayout {
    VkDescriptorSetLayout   vkDescriptorSetLayout;
    DescriptorSetLayoutData data;
};

struct Program {
    gerium_uint32_t                 activeShaders;
    VkPipelineShaderStageCreateInfo shaderStageInfo[kMaxShaderStages];
    std::vector<gerium_uint32_t>    spirv[kMaxShaderStages];
    bool                            graphicsPipeline;
    gerium_utf8_t                   name;

    absl::flat_hash_map<gerium_uint32_t, DescriptorSetLayoutData> descriptorSets;
};

struct Pipeline {
    VkPipeline                vkPipeline;
    VkPipelineLayout          vkPipelineLayout;
    VkPipelineBindPoint       vkBindPoint;
    RenderPassHandle          renderPass;
    gerium_uint32_t           numActiveLayouts;
    DescriptorSetLayoutHandle descriptorSetLayoutHandles[kMaxDescriptorSetLayouts];
};

struct Framebuffer {
    VkFramebuffer    vkFramebuffer;
    RenderPassHandle renderPass;
    gerium_uint16_t  width;
    gerium_uint16_t  height;
    gerium_uint16_t  layers;
    gerium_float32_t scaleX;
    gerium_float32_t scaleY;
    gerium_uint32_t  numColorAttachments;
    TextureHandle    colorAttachments[kMaxImageOutputs];
    TextureHandle    depthStencilAttachment;
    gerium_uint8_t   resize;
    gerium_utf8_t    name;
};

struct TechniquePass {
    gerium_utf8_t  renderPass;
    PipelineHandle pipeline;
};

struct Technique {
    gerium_utf8_t   name;
    gerium_uint32_t passCount;
    TechniquePass   passes[kMaxTechniquePasses];
};

// clang-format on

} // namespace gerium::vulkan

#endif
