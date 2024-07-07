#ifndef GERIUM_WINDOWS_VULKAN_RESOURCES_HPP
#define GERIUM_WINDOWS_VULKAN_RESOURCES_HPP

#include "../Gerium.hpp"
#include "../Handles.hpp"
#include "Enums.hpp"

namespace gerium::vulkan {

// clang-format off

constexpr uint8_t kMaxImageOutputs         = 8;
constexpr uint8_t kMaxDescriptorSetLayouts = 8;
constexpr uint8_t kMaxDescriptorsPerSet    = 16;
constexpr uint8_t kMaxVertexStreams        = 16;
constexpr uint8_t kMaxVertexAttributes     = 16;
constexpr uint8_t kMaxShaderStages         = 5;

struct RenderPassHandle : Handle {};
struct DescriptorSetLayoutHandle : Handle {};
struct ProgramHandle : Handle {};
struct PipelineHandle : Handle {};

using TexturePool             = ResourcePool<struct Texture, TextureHandle>;
using RenderPassPool          = ResourcePool<struct RenderPass, RenderPassHandle>;
using DescriptorSetLayoutPool = ResourcePool<struct DescriptorSetLayout, DescriptorSetLayoutHandle>;
using ProgramPool             = ResourcePool<struct Program, ProgramHandle>;
using PipelinePool            = ResourcePool<struct Pipeline, PipelineHandle>;
using FramebufferPool         = ResourcePool<struct Framebuffer, FramebufferHandle>;

struct TextureCreation {
    uint16_t              width        = 1;
    uint16_t              height       = 1;
    uint16_t              depth        = 1;
    uint16_t              mipmaps      = 1;
    TextureFlags          flags        = TextureFlags::None;
    gerium_format_t       format       = GERIUM_FORMAT_R8G8B8A8_UNORM;
    gerium_texture_type_t type         = GERIUM_TEXTURE_TYPE_2D;
    TextureHandle         alias        = Undefined;
    void*                 initialData  = nullptr;
    const char*           name         = nullptr;
    
    TextureCreation& setSize(uint16_t width, uint16_t height, uint16_t depth) {
        this->width  = width;
        this->height = height;
        this->depth  = depth;
        return *this;
    }

    TextureCreation& setFlags(uint8_t mipmaps, bool renderTarget, bool compute) {
        this->mipmaps = mipmaps;
        this->flags  |= renderTarget ? TextureFlags::RenderTarget : TextureFlags::None;
        this->flags  |= compute ? TextureFlags::Compute : TextureFlags::None;
        return *this;
    }

    TextureCreation& setFormat(gerium_format_t format, gerium_texture_type_t type) {
        this->format = format;
        this->type   = type;
        return *this;
    }

    TextureCreation& setAlias(TextureHandle alias) {
        this->alias = alias;
        return *this;
    }

    TextureCreation& setData(void* data) {
        this->initialData = data;
        return *this;
    }

    TextureCreation& setName(const char* name) {
        this->name = name;
        return *this;
    }
};

struct RenderPassOutput {
    uint32_t            numColorFormats;
    VkFormat            colorFormats[kMaxImageOutputs];
    VkImageLayout       colorFinalLayouts[kMaxImageOutputs];
    RenderPassOperation colorOperations[kMaxImageOutputs];

    VkFormat            depthStencilFormat;
    VkImageLayout       depthStencilFinalLayout;
    RenderPassOperation depthOperation;
    RenderPassOperation stencilOperation;

    RenderPassOutput& color(VkFormat format, VkImageLayout layout, RenderPassOperation loadOp) {
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

    RenderPassOutput& setDepthStencilOperations(RenderPassOperation depth, RenderPassOperation stencil) {
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

struct RasterizationCreation {
    VkCullModeFlagBits cullMode = VK_CULL_MODE_NONE;
    VkFrontFace        front    = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    FillMode           fill     = FillMode::Solid;
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

struct BlendState {
    VkBlendFactor sourceColor      = VK_BLEND_FACTOR_ONE;
    VkBlendFactor destinationColor = VK_BLEND_FACTOR_ONE;
    VkBlendOp     colorOperation   = VK_BLEND_OP_ADD;

    VkBlendFactor sourceAlpha      = VK_BLEND_FACTOR_ONE;
    VkBlendFactor destinationAlpha = VK_BLEND_FACTOR_ONE;
    VkBlendOp     alphaOperation   = VK_BLEND_OP_ADD;

    ColorWriteEnabled colorWriteMask = ColorWriteEnabled::All;

    uint8_t blendEnabled  : 1;
    uint8_t separateBlend : 1;
    uint8_t pad           : 6;

    BlendState() : blendEnabled(0), separateBlend(0) {
    }

    BlendState& setColor(VkBlendFactor sourceColor, VkBlendFactor destinationColor, VkBlendOp colorOperation) {
        this->sourceColor      = sourceColor;
        this->destinationColor = destinationColor;
        this->colorOperation   = colorOperation;
        this->blendEnabled     = 1;
        return *this;
    }

    BlendState& setAlpha(VkBlendFactor sourceAlpha, VkBlendFactor destinationAlpha, VkBlendOp alphaOperation) {
        sourceAlpha      = sourceAlpha;
        destinationAlpha = destinationAlpha;
        alphaOperation   = alphaOperation;
        separateBlend    = 1;
        return *this;
    }

    BlendState& setColorWriteMask(ColorWriteEnabled value) {
        colorWriteMask = value;
        return *this;
    }
};

struct BlendStateCreation {
    BlendState blendStates[kMaxImageOutputs];
    uint32_t   activeStates = 0;

    BlendStateCreation& reset() {
        activeStates = 0;
        return *this;
    }

    BlendState& addBlendState() {
        return blendStates[activeStates++];
    }
};

struct VertexStream {
    uint16_t        binding   = 0;
    uint16_t        stride    = 0;
    VertexInputRate inputRate = {}; // TODO:
};

struct VertexAttribute {
    uint16_t              location = 0;
    uint16_t              binding  = 0;
    uint32_t              offset   = 0;
    VertexComponentFormat format   = {}; // TODO:
};

struct VertexInputCreation {
    uint32_t numVertexStreams    = 0;
    uint32_t numVertexAttributes = 0;

    VertexStream    vertexStreams[kMaxVertexStreams];
    VertexAttribute vertexAttributes[kMaxVertexAttributes];

    VertexInputCreation& reset() {
        numVertexStreams = numVertexAttributes = 0;
        return *this;
    }

    VertexInputCreation& addVertexStream(const VertexStream& stream) {
        vertexStreams[numVertexStreams++] = stream;
        return *this;
    }

    VertexInputCreation& addVertexAttribute(const VertexAttribute& attribute) {
        vertexAttributes[numVertexAttributes++] = attribute;
        return *this;
    }
};

struct ShaderStage {
    const char*           code     = nullptr;
    uint32_t              codeSize = 0;
    VkShaderStageFlagBits type     = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
};

struct ProgramCreation {
    ShaderStage stages[kMaxShaderStages];

    const char* name = nullptr;

    uint32_t stagesCount = 0;
    uint32_t spvInput    = 0;

    // Building helpers
    ProgramCreation& reset() {
        stagesCount = 0;
        return *this;
    }

    ProgramCreation& setName(const char* name) {
        this->name = name;
        return *this;
    }

    ProgramCreation& addStage(const char* code, size_t codeSize, VkShaderStageFlagBits type) {
        stages[stagesCount].code     = code;
        stages[stagesCount].codeSize = (uint32_t) codeSize;
        stages[stagesCount].type     = type;
        ++stagesCount;
        return *this;
    }

    ProgramCreation& setSpvInput(bool value) {
        spvInput = value;
        return *this;
    }
};

struct Rect2DInt {
    int16_t  x      = 0;
    int16_t  y      = 0;
    uint16_t width  = 0;
    uint16_t height = 0;
};

struct Viewport {
    Rect2DInt rect;
    float     min_depth = 0.0f;
    float     max_depth = 0.0f;
};

struct ViewportState {
    uint32_t num_viewports = 0;
    uint32_t num_scissors  = 0;

    Viewport*  viewport = nullptr;
    Rect2DInt* scissors = nullptr;
};

struct PipelineCreation {
    RasterizationCreation rasterization;
    DepthStencilCreation  depthStencil;
    BlendStateCreation    blendState;
    VertexInputCreation   vertexInput;
    ProgramCreation       program;

    RenderPassOutput          renderPass;
    //DescriptorSetLayoutHandle descriptorSetLayout[kMaxDescriptorSetLayouts];
    const ViewportState*      viewport = nullptr;

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

/*struct DescriptorBinding {
    VkDescriptorType type;
    uint16_t         index;
    uint16_t         count;
    // uint16_t         set;

    const char* name;
};*/

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

    //SamplerHandle sampler;
};

struct RenderPass {
    VkRenderPass     vkRenderPass;
    RenderPassOutput output;
    gerium_utf8_t    name;
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

    absl::flat_hash_map<uint32_t, DescriptorSetLayoutData> descriptorSets;
};

struct Pipeline {
    VkPipeline       vkPipeline;
    VkPipelineLayout vkPipelineLayout;

    VkPipelineBindPoint vkBindPoint;

    ProgramHandle program;
    RenderPassHandle renderPass; // ???

    // const DescriptorSetLayout* descriptorSetLayout[k_max_descriptor_set_layouts];
    DescriptorSetLayoutHandle descriptorSetLayoutHandles[kMaxDescriptorSetLayouts];
    uint32_t                  numActiveLayouts;

    DepthStencilCreation  depthStencil;
    BlendStateCreation    blendState;
    RasterizationCreation rasterization;

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

// clang-format on

} // namespace gerium::vulkan

#endif
