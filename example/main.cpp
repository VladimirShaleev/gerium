#include <gerium/gerium.h>
#include <stdexcept>
#include <string>
#include <vector>

// GLM
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_FORCE_MESSAGES
// https://github.com/g-truc/glm/issues/1269
#include <glm/detail/setup.hpp>
#undef GLM_DEPRECATED
#define GLM_DEPRECATED [[deprecated]]
#include <glm/ext.hpp>

static gerium_application_t application = nullptr;
static gerium_logger_t logger           = nullptr;
static gerium_renderer_t renderer       = nullptr;
static gerium_frame_graph_t frameGraph  = nullptr;
static gerium_profiler_t profiler       = nullptr;

static gerium_buffer_h vertices                         = {};
static gerium_texture_h texture                         = {};
static gerium_buffer_h meshData0                        = {};
static gerium_buffer_h meshData1                        = {};
static gerium_buffer_h uniform1                         = {};
static gerium_buffer_h uniform2                         = {};
static gerium_technique_h baseTechnique                 = {};
static gerium_technique_h fullscreenTechnique           = {};
static gerium_descriptor_set_h descriptorSetMeshData[2] = {};
static gerium_descriptor_set_h descriptorSetUniform[2]  = {};
static gerium_descriptor_set_h descriptorSet1           = {};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct UniformBufferObject1 {
    float f;
};

struct UniformBufferObject2 {
    float f;
};

void check(gerium_result_t result) {
    if (result != GERIUM_RESULT_SUCCESS && result != GERIUM_RESULT_SKIP_FRAME) {
        throw std::runtime_error(gerium_result_to_string(result));
    }
}

/*  gerium_bool_t gbufferRender(gerium_frame_graph_t frame_graph, gerium_renderer_t renderer, gerium_data_t data) {
    return 1;
}

gerium_bool_t transparentRender(gerium_frame_graph_t frame_graph, gerium_renderer_t renderer, gerium_data_t data) {
    return 1;
}

gerium_bool_t depthOfFieldRender(gerium_frame_graph_t frame_graph, gerium_renderer_t renderer, gerium_data_t data) {
    return 1;
}

gerium_bool_t lightingRender(gerium_frame_graph_t frame_graph, gerium_renderer_t renderer, gerium_data_t data) {
    return 1;
}

gerium_bool_t depthPrePassRender(gerium_frame_graph_t frame_graph, gerium_renderer_t renderer, gerium_data_t data) {
    return 1;
} */

gerium_uint32_t simplePrepare(gerium_frame_graph_t frame_graph, gerium_renderer_t renderer, gerium_data_t data) {
    return 2;
}

gerium_bool_t simpleRender(gerium_frame_graph_t frame_graph,
                           gerium_renderer_t renderer,
                           gerium_command_buffer_t command_buffer,
                           gerium_uint32_t worker,
                           gerium_uint32_t total_workers,
                           gerium_data_t data) {
    gerium_command_buffer_bind_technique(command_buffer, baseTechnique);
    gerium_command_buffer_bind_descriptor_set(command_buffer, descriptorSetMeshData[worker], 0);
    gerium_command_buffer_bind_descriptor_set(command_buffer, descriptorSetUniform[worker], 1);
    gerium_command_buffer_bind_vertex_buffer(command_buffer, vertices, 0, 0);
    gerium_command_buffer_draw(command_buffer, 0, 3, 0, 1);
    return 1;
}

gerium_bool_t fullscreenRender(gerium_frame_graph_t frame_graph,
                               gerium_renderer_t renderer,
                               gerium_command_buffer_t command_buffer,
                               gerium_uint32_t worker,
                               gerium_uint32_t total_workers,
                               gerium_data_t data) {
    gerium_command_buffer_bind_technique(command_buffer, fullscreenTechnique);
    gerium_command_buffer_bind_descriptor_set(command_buffer, descriptorSet1, 0);
    gerium_command_buffer_draw(command_buffer, 0, 3, 0, 1);
    gerium_command_buffer_draw_profiler(command_buffer, nullptr);
    return 1;
}

bool initialize(gerium_application_t application) {
    try {
        constexpr auto debug =
#ifdef NDEBUG
            false;
#else
            true;
#endif

        check(gerium_renderer_create(application, GERIUM_VERSION_ENCODE(1, 0, 0), debug, &renderer));
        gerium_renderer_set_profiler_enable(renderer, 1);

        check(gerium_frame_graph_create(renderer, &frameGraph));
        check(gerium_profiler_create(renderer, &profiler));

        struct Vertex {
            float position[2];
            float color[3];
            float texcoord[2];
        };

        Vertex vData[3]{
            { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.5f, 1.0f } },
            { { 0.5, 0.5 },    { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
            { { -0.5, 0.5 },   { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }
        };
        check(gerium_renderer_create_buffer(
            renderer, GERIUM_BUFFER_USAGE_VERTEX_BIT, 0, "vertices", vData, sizeof(Vertex) * 3, &vertices));

        std::vector<gerium_uint8_t> texData;
        texData.resize(64 * 64 * 4);

        for (gerium_sint32_t y = 0; y < 64; ++y) {
            for (gerium_sint32_t x = 0; x < 64; ++x) {
                auto pos  = (y * 64 + x) * 4;
                auto pX   = x - 32;
                auto pY   = y - 32;
                auto dist = std::sqrt(pX * pX + pY * pY);
                dist      = (dist > 32 ? 32 : dist) / 32;
                auto c    = 1 - dist * dist * dist;
                c         = c * 255;
                auto cc   = uint32_t(c);
                if (cc > 255) {
                    cc = 255;
                } else if (c < 0) {
                    c = 0;
                }
                texData[pos + 0] = cc;
                texData[pos + 1] = cc;
                texData[pos + 2] = cc;
                texData[pos + 3] = 255;
            }
        }

        gerium_texture_creation_t c{};
        c.width   = 64;
        c.height  = 64;
        c.depth   = 1;
        c.mipmaps = 1;
        c.format  = GERIUM_FORMAT_R8G8B8A8_UNORM;
        c.type    = GERIUM_TEXTURE_TYPE_2D;
        c.data    = (gerium_cdata_t) texData.data();
        c.name    = "texture";
        check(gerium_renderer_create_texture(renderer, &c, &texture));

        check(gerium_renderer_create_buffer(renderer,
                                            GERIUM_BUFFER_USAGE_UNIFORM_BIT,
                                            1,
                                            "mesh_data",
                                            nullptr,
                                            sizeof(UniformBufferObject),
                                            &meshData0));

        check(gerium_renderer_create_buffer(renderer,
                                            GERIUM_BUFFER_USAGE_UNIFORM_BIT,
                                            1,
                                            "mesh_data",
                                            nullptr,
                                            sizeof(UniformBufferObject),
                                            &meshData1));

        check(gerium_renderer_create_buffer(renderer,
                                            GERIUM_BUFFER_USAGE_UNIFORM_BIT,
                                            1,
                                            "uniform1",
                                            nullptr,
                                            sizeof(UniformBufferObject1),
                                            &uniform1));

        check(gerium_renderer_create_buffer(renderer,
                                            GERIUM_BUFFER_USAGE_UNIFORM_BIT,
                                            1,
                                            "uniform2",
                                            nullptr,
                                            sizeof(UniformBufferObject2),
                                            &uniform2));

        check(gerium_renderer_create_descriptor_set(renderer, &descriptorSetMeshData[0]));
        check(gerium_renderer_create_descriptor_set(renderer, &descriptorSetMeshData[1]));
        check(gerium_renderer_create_descriptor_set(renderer, &descriptorSetUniform[0]));
        check(gerium_renderer_create_descriptor_set(renderer, &descriptorSetUniform[1]));
        check(gerium_renderer_create_descriptor_set(renderer, &descriptorSet1));

        gerium_renderer_bind_buffer(renderer, descriptorSetMeshData[0], 0, meshData0);
        gerium_renderer_bind_texture(renderer, descriptorSetMeshData[0], 1, texture);
        gerium_renderer_bind_buffer(renderer, descriptorSetMeshData[0], 2, uniform2);
        gerium_renderer_bind_buffer(renderer, descriptorSetUniform[0], 2, uniform1);
        gerium_renderer_bind_buffer(renderer, descriptorSetMeshData[1], 0, meshData1);
        gerium_renderer_bind_texture(renderer, descriptorSetMeshData[1], 1, texture);
        gerium_renderer_bind_buffer(renderer, descriptorSetMeshData[1], 2, uniform2);
        gerium_renderer_bind_buffer(renderer, descriptorSetUniform[1], 2, uniform1);
        gerium_renderer_bind_resource(renderer, descriptorSet1, 0, "color");

        /*gerium_render_pass_t gbufferPass{ 0, 0, gbufferRender };
        gerium_render_pass_t transparentPass{ 0, 0, transparentRender };
        gerium_render_pass_t depthOfFieldPass{ 0, 0, depthOfFieldRender };
        gerium_render_pass_t lightingPass{ 0, 0, lightingRender };
        gerium_render_pass_t depthPrePass{ 0, 0, depthPrePassRender };
        gerium_frame_graph_add_pass(frameGraph, "gbuffer_pass", &gbufferPass, nullptr);
        gerium_frame_graph_add_pass(frameGraph, "transparent_pass", &transparentPass, nullptr);
        gerium_frame_graph_add_pass(frameGraph, "depth_of_field_pass", &depthOfFieldPass, nullptr);
        gerium_frame_graph_add_pass(frameGraph, "lighting_pass", &lightingPass, nullptr);
        gerium_frame_graph_add_pass(frameGraph, "depth_pre_pass", &depthPrePass, nullptr);

        gerium_resource_input_t gbufferInputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT, "depth" }
        };
        gerium_resource_output_t gbufferOutputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "color",                        0,
             GERIUM_FORMAT_R8G8B8A8_UNORM,      0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR },
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "normals",                      0,
             GERIUM_FORMAT_R16G16B16A16_SFLOAT, 0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR },
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "metallic_roughness_occlusion", 0,
             GERIUM_FORMAT_R8G8B8A8_UNORM,      0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR },
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "position",                     0,
             GERIUM_FORMAT_R16G16B16A16_SFLOAT, 0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR },
        };

        gerium_resource_input_t transparentInputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT, "lighting" },
            { GERIUM_RESOURCE_TYPE_ATTACHMENT, "depth"    }
        };
        gerium_resource_output_t transparentOutputs[] = {
            { GERIUM_RESOURCE_TYPE_REFERENCE, "lighting" }
        };

        gerium_resource_input_t dofInputs[] = {
            { GERIUM_RESOURCE_TYPE_REFERENCE, "lighting" },
            { GERIUM_RESOURCE_TYPE_REFERENCE, "depth"    },
        };
        gerium_resource_output_t dofOutputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "final", 0,
             GERIUM_FORMAT_R8G8B8A8_UNORM, 0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR }
        };

        gerium_resource_input_t lightingInputs[] = {
            { GERIUM_RESOURCE_TYPE_TEXTURE, "color"                        },
            { GERIUM_RESOURCE_TYPE_TEXTURE, "normals"                      },
            { GERIUM_RESOURCE_TYPE_TEXTURE, "metallic_roughness_occlusion" },
            { GERIUM_RESOURCE_TYPE_TEXTURE, "position"                     }
        };
        gerium_resource_output_t lightingOutputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "lighting", 0,
             GERIUM_FORMAT_R8G8B8A8_UNORM, 0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR }
        };

        gerium_resource_output_t depthOutputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "depth", 0,
             GERIUM_FORMAT_D32_SFLOAT, 0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR }
        };

        check(gerium_frame_graph_add_node(frameGraph,
                                          "gbuffer_pass",
                                          std::size(gbufferInputs),
                                          gbufferInputs,
                                          std::size(gbufferOutputs),
                                          gbufferOutputs));
        check(gerium_frame_graph_add_node(frameGraph,
                                          "transparent_pass",
                                          std::size(transparentInputs),
                                          transparentInputs,
                                          std::size(transparentOutputs),
                                          transparentOutputs));
        check(gerium_frame_graph_add_node(
            frameGraph, "depth_of_field_pass", std::size(dofInputs), dofInputs, std::size(dofOutputs), dofOutputs));
        check(gerium_frame_graph_add_node(frameGraph,
                                          "lighting_pass",
                                          std::size(lightingInputs),
                                          lightingInputs,
                                          std::size(lightingOutputs),
                                          lightingOutputs));
        check(gerium_frame_graph_add_node(
            frameGraph, "depth_pre_pass", 0, nullptr, std::size(depthOutputs), depthOutputs));
        check(gerium_frame_graph_compile(frameGraph));

        gerium_vertex_attribute_t vertexAttributes[] = {
            { 0, 0, 0,                                     GERIUM_FORMAT_R32G32_SFLOAT    },
            { 1, 0, sizeof(float) * 2,                     GERIUM_FORMAT_R32G32B32_SFLOAT },
            { 2, 0, sizeof(float) * 2 + sizeof(float) * 3, GERIUM_FORMAT_R32G32_SFLOAT    }
        };

        gerium_vertex_binding_t vertexBindings[] = {
            { 0, sizeof(float) * 7, GERIUM_VERTEX_RATE_PER_VERTEX }
        };

        gerium_shader_t lightingShaders[2];
        lightingShaders[0].type = GERIUM_SHADER_TYPE_VERTEX;
        lightingShaders[0].name = "lighting.vert.glsl";
        lightingShaders[0].data = "#version 450\n"
                                  "\n"
                                  "layout(location = 0) in vec2 inPosition;\n"
                                  "layout(location = 1) in vec3 inColor;\n"
                                  "layout(location = 2) in vec2 inTexCoord;\n"
                                  "\n"
                                  "layout(location = 0) out vec3 outColor;\n"
                                  "layout(location = 1) out vec2 outTexCoord;\n"
                                  "\n"
                                  "layout(binding = 0) uniform UniformBufferObject {\n"
                                  "    mat4 model;\n"
                                  "    mat4 view;\n"
                                  "    mat4 proj;\n"
                                  "} ubo;\n"
                                  "\n"
                                  "void main() {\n"
                                  "    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);\n"
                                  "    outColor = inColor;\n"
                                  "    outTexCoord = inTexCoord;\n"
                                  "}\n";
        lightingShaders[0].size = strlen(lightingShaders[0].data);

        lightingShaders[1].type = GERIUM_SHADER_TYPE_FRAGMENT;
        lightingShaders[1].name = "lighting.frag.glsl";
        lightingShaders[1].data = "#version 450\n"
                                  "\n"
                                  "layout(location = 0) in vec3 inColor;\n"

                                  "layout(location = 0) out vec4 outColor;\n"
                                  "\n"
                                  "layout(set = 1, binding = 2) uniform UniformBufferObject1 {\n"
                                  "    float f;\n"
                                  "} test;\n"
                                  "\n"
                                  "layout(set = 0, binding = 2) uniform UniformBufferObject2 {\n"
                                  "    float f;\n"
                                  "} test2;\n"
                                  "\n"
                                  "void main() {\n"
                                  "    outColor = vec4(inColor.r * test.f, inColor.g * test2.f, inColor.b, 1.0);\n"
                                  "}\n";
        lightingShaders[1].size = strlen(lightingShaders[0].data);

        gerium_pipeline_t lightingPipelines[1];
        lightingPipelines[0].render_pass            = "lighting_pass";
        lightingPipelines[0].vertex_attribute_count = std::size(vertexAttributes);
        lightingPipelines[0].vertex_attributes      = vertexAttributes;
        lightingPipelines[0].vertex_binding_count   = std::size(vertexBindings);
        lightingPipelines[0].vertex_bindings        = vertexBindings;
        lightingPipelines[0].shader_count           = std::size(lightingShaders);
        lightingPipelines[0].shaders                = lightingShaders;

        check(gerium_renderer_create_technique(
            renderer, frameGraph, "lighting", std::size(lightingPipelines), lightingPipelines, &lighting));*/

        gerium_render_pass_t fullscreenPass{ 0, 0, fullscreenRender };
        gerium_frame_graph_add_pass(frameGraph, "present_pass", &fullscreenPass, nullptr);

        gerium_render_pass_t simplePass{ simplePrepare, 0, simpleRender };
        gerium_frame_graph_add_pass(frameGraph, "simple_pass", &simplePass, nullptr);

        gerium_resource_input_t fullscreenInputs[] = {
            { GERIUM_RESOURCE_TYPE_TEXTURE, "color" },
        };
        check(gerium_frame_graph_add_node(
            frameGraph, "present_pass", std::size(fullscreenInputs), fullscreenInputs, 0, nullptr));

        gerium_resource_output_t simpleOutputs[2]{};
        simpleOutputs[0].type             = GERIUM_RESOURCE_TYPE_ATTACHMENT;
        simpleOutputs[0].name             = "color";
        simpleOutputs[0].format           = GERIUM_FORMAT_R8G8B8A8_UNORM;
        simpleOutputs[0].auto_scale       = 1.0f;
        simpleOutputs[0].render_pass_op   = GERIUM_RENDER_PASS_OP_CLEAR;
        simpleOutputs[0].color_write_mask = GERIUM_COLOR_COMPONENT_R_BIT | GERIUM_COLOR_COMPONENT_G_BIT |
                                            GERIUM_COLOR_COMPONENT_B_BIT | GERIUM_COLOR_COMPONENT_A_BIT;

        simpleOutputs[1].type           = GERIUM_RESOURCE_TYPE_ATTACHMENT;
        simpleOutputs[1].name           = "depth";
        simpleOutputs[1].format         = GERIUM_FORMAT_D24_UNORM_S8_UINT;
        simpleOutputs[1].auto_scale     = 1.0f;
        simpleOutputs[1].render_pass_op = GERIUM_RENDER_PASS_OP_CLEAR;

        simpleOutputs[0].clear_color_attachment         = { 1.0f, 0.0f, 1.0f, 1.0f };
        simpleOutputs[1].clear_depth_stencil_attachment = { 1.0f, 0 };

        check(gerium_frame_graph_add_node(
            frameGraph, "simple_pass", 0, nullptr, std::size(simpleOutputs), simpleOutputs));

        check(gerium_frame_graph_compile(frameGraph));

        gerium_vertex_attribute_t vertexAttributes[] = {
            { 0, 0, 0,                                     GERIUM_FORMAT_R32G32_SFLOAT    },
            { 1, 0, sizeof(float) * 2,                     GERIUM_FORMAT_R32G32B32_SFLOAT },
            { 2, 0, sizeof(float) * 2 + sizeof(float) * 3, GERIUM_FORMAT_R32G32_SFLOAT    }
        };

        gerium_vertex_binding_t vertexBindings[] = {
            { 0, sizeof(float) * 7, GERIUM_VERTEX_RATE_PER_VERTEX }
        };

        gerium_shader_t baseShaders[2]{};
        baseShaders[0].type = GERIUM_SHADER_TYPE_VERTEX;
        baseShaders[0].lang = GERIUM_SHADER_LANGUAGE_GLSL;
        baseShaders[0].name = "base.vert.glsl";
        baseShaders[0].data = "#version 450\n"
                              "\n"
                              "layout(location = 0) in vec2 inPosition;\n"
                              "layout(location = 1) in vec3 inColor;\n"
                              "layout(location = 2) in vec2 inTexCoord;\n"
                              "\n"
                              "layout(location = 0) out vec3 outColor;\n"
                              "layout(location = 1) out vec2 outTexCoord;\n"
                              "\n"
                              "layout(binding = 0) uniform UniformBufferObject {\n"
                              "    mat4 model;\n"
                              "    mat4 view;\n"
                              "    mat4 proj;\n"
                              "} ubo;\n"
                              "\n"
                              "void main() {\n"
                              "    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);\n"
                              "    outColor = inColor;\n"
                              "    outTexCoord = inTexCoord;\n"
                              "}\n";
        baseShaders[0].size = strlen((const char*) baseShaders[0].data);

        baseShaders[1].type = GERIUM_SHADER_TYPE_FRAGMENT;
        baseShaders[1].lang = GERIUM_SHADER_LANGUAGE_GLSL;
        baseShaders[1].name = "base.frag.glsl";
        baseShaders[1].data = "#version 450\n"
                              "\n"
                              "layout(location = 0) in vec3 inColor;\n"
                              "layout(location = 1) in vec2 inTexCoord;\n"

                              "layout(location = 0) out vec4 outColor;\n"
                              "\n"
                              "layout(binding = 1) uniform sampler2D texColor;\n"
                              "\n"
                              "layout(set = 1, binding = 2) uniform UniformBufferObject1 {\n"
                              "    float f;\n"
                              "} test;\n"
                              "\n"
                              "layout(set = 0, binding = 2) uniform UniformBufferObject2 {\n"
                              "    float f;\n"
                              "} test2;\n"
                              "\n"
                              "void main() {\n"
                              "    vec3 color = texture(texColor, inTexCoord).rgb;\n"
                              "    outColor = vec4(inColor.r * test.f * color.r, inColor.g * test2.f * color.g, "
                              "inColor.b * color.b, 1.0);\n"
                              "}\n";
        baseShaders[1].size = strlen((const char*) baseShaders[1].data);

        gerium_color_blend_state_t colorBlend{};
        gerium_depth_stencil_state_t depthStencil{};
        gerium_rasterization_state_t rasterization{};
        rasterization.polygon_mode               = GERIUM_POLYGON_MODE_FILL;
        rasterization.cull_mode                  = GERIUM_CULL_MODE_NONE;
        rasterization.front_face                 = GERIUM_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization.depth_clamp_enable         = 0;
        rasterization.depth_bias_enable          = 0;
        rasterization.depth_bias_constant_factor = 0.0f;
        rasterization.depth_bias_clamp           = 0.0f;
        rasterization.depth_bias_slope_factor    = 0.0f;
        rasterization.line_width                 = 1.0f;

        gerium_pipeline_t basePipelines[1]{};
        basePipelines[0].render_pass            = "simple_pass";
        basePipelines[0].rasterization          = &rasterization;
        basePipelines[0].depth_stencil          = &depthStencil;
        basePipelines[0].color_blend            = &colorBlend;
        basePipelines[0].vertex_attribute_count = std::size(vertexAttributes);
        basePipelines[0].vertex_attributes      = vertexAttributes;
        basePipelines[0].vertex_binding_count   = std::size(vertexBindings);
        basePipelines[0].vertex_bindings        = vertexBindings;
        basePipelines[0].shader_count           = std::size(baseShaders);
        basePipelines[0].shaders                = baseShaders;

        check(gerium_renderer_create_technique(
            renderer, frameGraph, "base", std::size(basePipelines), basePipelines, &baseTechnique));

        gerium_shader_t fullscreenShaders[2]{};
        fullscreenShaders[0].type = GERIUM_SHADER_TYPE_VERTEX;
        fullscreenShaders[0].lang = GERIUM_SHADER_LANGUAGE_HLSL;
        fullscreenShaders[0].name = "fullscreen.vert.hlsl";
        // fullscreenShaders[0].data = "#version 450\n"
        //                             "\n"
        //                             "layout(location = 0) out vec2 vTexCoord;"
        //                             "\n"
        //                             "void main() {\n"
        //                             "    vTexCoord.xy = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);\n"
        //                             "    gl_Position = vec4(vTexCoord.xy * 2.0f - 1.0f, 0.0f, 1.0f);\n"
        //                             "    gl_Position.y = -gl_Position.y;\n"
        //                             "}\n";
        fullscreenShaders[0].data = "struct Output {\n"
                                    "    float4 pos : SV_POSITION;\n"
                                    "    float2 uv  : TEXCOORD0;\n"
                                    "};\n"
                                    "\n"
                                    "Output main(uint id : SV_VertexID) {\n"
                                    "    Output output;\n"
                                    "    output.uv.x = float((id << 1) & 2);\n"
                                    "    output.uv.y = float(id & 2);\n"
                                    "    output.pos.xy = output.uv.xy * 2.0f - 1.0f;\n"
                                    "    output.pos.z = 0.0f;\n"
                                    "    output.pos.w = 1.0f;\n"
                                    "    output.pos.y = -output.pos.y;\n"
                                    "    return output;\n"
                                    "}\n";
        fullscreenShaders[0].size = strlen((const char*) fullscreenShaders[0].data);

        fullscreenShaders[1].type = GERIUM_SHADER_TYPE_FRAGMENT;
        fullscreenShaders[1].lang = GERIUM_SHADER_LANGUAGE_GLSL;
        fullscreenShaders[1].name = "fullscreen.frag.glsl";
        fullscreenShaders[1].data = "#version 450\n"
                                    "\n"
                                    "layout(location = 0) in vec2 vTexCoord;\n"

                                    "layout(binding = 0) uniform sampler2D texColor;\n"

                                    "layout(location = 0) out vec4 outColor;\n"
                                    "\n"
                                    "void main() {\n"
                                    "    outColor = texture(texColor, vTexCoord);\n"
                                    "}\n";
        fullscreenShaders[1].size = strlen((const char*) fullscreenShaders[1].data);

        gerium_pipeline_t fullscreenPipelines[1]{};
        fullscreenPipelines[0].render_pass   = "present_pass";
        fullscreenPipelines[0].rasterization = &rasterization;
        fullscreenPipelines[0].depth_stencil = &depthStencil;
        fullscreenPipelines[0].color_blend   = &colorBlend;
        fullscreenPipelines[0].shader_count  = std::size(fullscreenShaders);
        fullscreenPipelines[0].shaders       = fullscreenShaders;

        check(gerium_renderer_create_technique(renderer,
                                               frameGraph,
                                               "fullscreen",
                                               std::size(fullscreenPipelines),
                                               fullscreenPipelines,
                                               &fullscreenTechnique));

    } catch (const std::runtime_error& exc) {
        gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_FATAL, exc.what());
        return false;
    } catch (...) {
        gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_FATAL, "unknown error");
        return false;
    }
    return true;
}

void unitialize(gerium_application_t application) {
    if (renderer) {
        gerium_renderer_destroy_descriptor_set(renderer, descriptorSet1);
        gerium_renderer_destroy_descriptor_set(renderer, descriptorSetUniform[1]);
        gerium_renderer_destroy_descriptor_set(renderer, descriptorSetUniform[0]);
        gerium_renderer_destroy_descriptor_set(renderer, descriptorSetMeshData[1]);
        gerium_renderer_destroy_descriptor_set(renderer, descriptorSetMeshData[0]);
        gerium_renderer_destroy_technique(renderer, fullscreenTechnique);
        gerium_renderer_destroy_technique(renderer, baseTechnique);
        gerium_renderer_destroy_buffer(renderer, uniform2);
        gerium_renderer_destroy_buffer(renderer, uniform1);
        gerium_renderer_destroy_buffer(renderer, meshData1);
        gerium_renderer_destroy_buffer(renderer, meshData0);
        gerium_renderer_destroy_texture(renderer, texture);
        gerium_renderer_destroy_buffer(renderer, vertices);
    }

    gerium_profiler_destroy(profiler);
    gerium_frame_graph_destroy(frameGraph);
    gerium_renderer_destroy(renderer);
}

gerium_bool_t frame(gerium_application_t application, gerium_data_t data, gerium_float32_t elapsed) {
    if (gerium_renderer_new_frame(renderer) == GERIUM_RESULT_SKIP_FRAME) {
        return 1;
    }

    gerium_uint16_t width, height;
    gerium_application_get_size(application, &width, &height);

    static float f1 = 1.0f;
    static float f2 = 0.5f;
    static float d1 = -0.001f;
    static float d2 = 0.001f;

    static float posY = 0.0f;
    static float posX = -0.5f;

    glm::vec2 moveDir(0.0f, 0.0f);

    if (gerium_application_is_press_scancode(application, GERIUM_SCANCODE_A)) {
        moveDir.x -= 1;
    }
    if (gerium_application_is_press_scancode(application, GERIUM_SCANCODE_D)) {
        moveDir.x += 1;
    }
    if (gerium_application_is_press_scancode(application, GERIUM_SCANCODE_W)) {
        moveDir.y += 1;
    }
    if (gerium_application_is_press_scancode(application, GERIUM_SCANCODE_S)) {
        moveDir.y -= 1;
    }

    auto move = glm::normalize(moveDir);

    if (!std::isnan(move.x)) {
        move *= 0.001f * elapsed;
        posX += move.x;
        posY += move.y;
    }

    auto transforms   = (UniformBufferObject*) gerium_renderer_map_buffer(renderer, meshData0, 0, 0);
    transforms->model = glm::translate(glm::vec3(posX, posY, 0.0f));
    transforms->view  = glm::translate(glm::vec3(0.0f, 0.0f, -2.0f * f1));
    transforms->proj  = glm::perspective(glm::radians(60.0f), float(width) / height, 0.1f, 1000.0f);
    gerium_renderer_unmap_buffer(renderer, meshData0);

    transforms        = (UniformBufferObject*) gerium_renderer_map_buffer(renderer, meshData1, 0, 0);
    transforms->model = glm::translate(glm::vec3(0.5f, 0.0f, 0.0f));
    transforms->view  = glm::translate(glm::vec3(0.0f, 0.0f, -2.0f * f2));
    transforms->proj  = glm::perspective(glm::radians(60.0f), float(width) / height, 0.1f, 1000.0f);
    gerium_renderer_unmap_buffer(renderer, meshData1);

    auto data1 = (UniformBufferObject1*) gerium_renderer_map_buffer(renderer, uniform1, 0, 0);
    data1->f   = f1;
    gerium_renderer_unmap_buffer(renderer, uniform1);

    auto data2 = (UniformBufferObject2*) gerium_renderer_map_buffer(renderer, uniform2, 0, 0);
    data2->f   = f2;
    gerium_renderer_unmap_buffer(renderer, uniform2);

    gerium_renderer_render(renderer, frameGraph);
    gerium_renderer_present(renderer);

    // f1 += d1 * elapsed;
    f2 += d2 * elapsed;

    if (f1 < 0.5f) {
        f1 = 0.5f;
        d1 = 0.001f;
    }
    if (f1 > 1.0f) {
        f1 = 1.0f;
        d1 = -0.001f;
    }

    if (f2 < 0.5f) {
        f2 = 0.5f;
        d2 = 0.001f;
    }
    if (f2 > 1.0f) {
        f2 = 1.0f;
        d2 = -0.001f;
    }

    return 1;
}

gerium_bool_t state(gerium_application_t application, gerium_data_t data, gerium_application_state_t state) {
    switch (state) {
        case GERIUM_APPLICATION_STATE_CREATE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_CREATE");
            break;
        case GERIUM_APPLICATION_STATE_DESTROY:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_DESTROY");
            break;
        case GERIUM_APPLICATION_STATE_INITIALIZE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_INITIALIZE");
            return initialize(application) ? 1 : 0;
        case GERIUM_APPLICATION_STATE_UNINITIALIZE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_UNINITIALIZE");
            unitialize(application);
            break;
        case GERIUM_APPLICATION_STATE_GOT_FOCUS:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_GOT_FOCUS");
            break;
        case GERIUM_APPLICATION_STATE_LOST_FOCUS:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_LOST_FOCUS");
            break;
        case GERIUM_APPLICATION_STATE_VISIBLE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_VISIBLE");
            break;
        case GERIUM_APPLICATION_STATE_INVISIBLE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_INVISIBLE");
            break;
        case GERIUM_APPLICATION_STATE_NORMAL:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_NORMAL");
            break;
        case GERIUM_APPLICATION_STATE_MINIMIZE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_MINIMIZE");
            break;
        case GERIUM_APPLICATION_STATE_MAXIMIZE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_MAXIMIZE");
            break;
        case GERIUM_APPLICATION_STATE_FULLSCREEN:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_FULLSCREEN");
            break;
        case GERIUM_APPLICATION_STATE_RESIZE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_RESIZE");
            break;
        case GERIUM_APPLICATION_STATE_RESIZED:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_RESIZED");
            break;
    }
    return 1;
}

#ifdef GERIUM_PLATFORM_WINDOWS
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main() {
#endif
    // Testing file
    // const auto a1 = gerium_file_get_cache_dir();
    // const auto a2 = gerium_file_get_app_dir();
    // const auto b1 = gerium_file_exists_file("D:\\Development\\Projects\\gerium\\example\\main.cpp"); //
    // "/storage/emulated/0/Pictures/IMG_20240725_013124.jpg"); const auto b2 =
    // gerium_file_exists_file("D:\\Development\\Projects\\gerium\\example"); // ""/storage/emulated/0/Pictures"); const
    // auto b3 = gerium_file_exists_dir("D:\\Development\\Projects\\gerium\\example\\main.cpp"); //
    // ""/storage/emulated/0/Pictures/IMG_20240725_013124.jpg"); const auto b4 =
    // gerium_file_exists_dir("D:\\Development\\Projects\\gerium\\example\\"); // ""/storage/emulated/0/Pictures");

    // gerium_file_t file;
    // gerium_file_create_temp(1024 * 1024 * 4, &file);

    // const auto size = gerium_file_get_size(file);

    // char* data = (char*) gerium_file_map(file);
    // *data++    = 'a';
    // *data++    = 'b';
    // *data++    = 'c';
    // *data++    = 'd';

    // gerium_file_write(file, "12", 2);
    // gerium_file_seek(file, 0, GERIUM_FILE_SEEK_BEGIN);

    // char result[5]{};
    // auto s = gerium_file_read(file, result, 4);

    // gerium_file_destroy(file);

    check(gerium_logger_create("app", &logger));

    try {
        check(gerium_application_create("test", 800, 600, &application));
        gerium_application_set_background_wait(application, 1);

        gerium_display_info_t displays[10];
        gerium_uint32_t displayCount = 10;
        check(gerium_application_get_display_info(application, &displayCount, displays));

        gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_INFO, "Avaiable monitors:");
        for (gerium_uint32_t i = 0; i < displayCount; ++i) {
            std::string log = "\t";
            log += displays[i].device_name;
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_INFO, log.c_str());
        }

        gerium_application_set_frame_func(application, frame, nullptr);
        gerium_application_set_state_func(application, state, nullptr);
        check(gerium_application_run(application));

    } catch (const std::runtime_error& exc) {
        gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_FATAL, exc.what());
    } catch (...) {
        gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_FATAL, "unknown error");
    }
    gerium_application_destroy(application);
    gerium_logger_destroy(logger);
    return 0;
}
