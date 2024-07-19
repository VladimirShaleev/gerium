#include <gerium/gerium.h>
#include <stdexcept>
#include <string>

static gerium_application_t application = nullptr;
static gerium_logger_t logger           = nullptr;
static gerium_renderer_t renderer       = nullptr;
static gerium_frame_graph_t frameGraph  = nullptr;
static gerium_profiler_t profiler       = nullptr;

static gerium_buffer_h vertices               = {};
static gerium_material_h baseMaterial         = {};
static gerium_material_h fullscreenMaterial   = {};
static gerium_descriptor_set_h descriptorSet1 = {};

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

gerium_bool_t simpleRender(gerium_frame_graph_t frame_graph,
                           gerium_renderer_t renderer,
                           gerium_command_buffer_t command_buffer,
                           gerium_data_t data) {
    gerium_command_buffer_bind_material(command_buffer, baseMaterial);
    gerium_command_buffer_bind_vertex_buffer(command_buffer, vertices, 0, 0);
    gerium_command_buffer_draw(command_buffer, 0, 3, 0, 1);
    return 1;
}

gerium_bool_t fullscreenRender(gerium_frame_graph_t frame_graph,
                               gerium_renderer_t renderer,
                               gerium_command_buffer_t command_buffer,
                               gerium_data_t data) {
    gerium_command_buffer_bind_material(command_buffer, fullscreenMaterial);
    gerium_command_buffer_bind_descriptor_set(command_buffer, descriptorSet1, 0);
    gerium_command_buffer_draw(command_buffer, 0, 3, 0, 1);
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
            { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { 0.5, 0.5 },    { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
            { { -0.5, 0.5 },   { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }
        };
        check(gerium_renderer_create_buffer_from_data(renderer, "vertices", vData, sizeof(Vertex) * 3, &vertices));

        check(gerium_renderer_create_descriptor_set(renderer, &descriptorSet1));

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

        check(gerium_renderer_create_material(
            renderer, frameGraph, "lighting", std::size(lightingPipelines), lightingPipelines, &lighting));*/

        gerium_render_pass_t fullscreenPass{ 0, 0, fullscreenRender };
        gerium_frame_graph_add_pass(frameGraph, "present_pass", &fullscreenPass, nullptr);

        gerium_render_pass_t simplePass{ 0, 0, simpleRender };
        gerium_frame_graph_add_pass(frameGraph, "simple_pass", &simplePass, nullptr);

        gerium_resource_input_t fullscreenInputs[] = {
            { GERIUM_RESOURCE_TYPE_TEXTURE, "color" },
        };
        check(gerium_frame_graph_add_node(
            frameGraph, "present_pass", std::size(fullscreenInputs), fullscreenInputs, 0, nullptr));

        gerium_resource_output_t simpleOutputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "color", 0,
             GERIUM_FORMAT_R8G8B8A8_UNORM,    0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR },
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "depth", 0,
             GERIUM_FORMAT_D24_UNORM_S8_UINT, 0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR }
        };
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
                              "void main() {\n"
                              "    gl_Position = vec4(inPosition, 0.0, 1.0);\n"
                              "    outColor = inColor;\n"
                              "    outTexCoord = inTexCoord;\n"
                              "}\n";
        baseShaders[0].size = strlen(baseShaders[0].data);

        baseShaders[1].type = GERIUM_SHADER_TYPE_FRAGMENT;
        baseShaders[1].name = "base.frag.glsl";
        baseShaders[1].data = "#version 450\n"
                              "\n"
                              "layout(location = 0) in vec3 inColor;\n"

                              "layout(location = 0) out vec4 outColor;\n"
                              "\n"
                              "void main() {\n"
                              "    outColor = vec4(inColor, 1.0);\n"
                              "}\n";
        baseShaders[1].size = strlen(baseShaders[1].data);

        gerium_pipeline_t basePipelines[1];
        basePipelines[0].render_pass            = "simple_pass";
        basePipelines[0].vertex_attribute_count = std::size(vertexAttributes);
        basePipelines[0].vertex_attributes      = vertexAttributes;
        basePipelines[0].vertex_binding_count   = std::size(vertexBindings);
        basePipelines[0].vertex_bindings        = vertexBindings;
        basePipelines[0].shader_count           = std::size(baseShaders);
        basePipelines[0].shaders                = baseShaders;

        check(gerium_renderer_create_material(
            renderer, frameGraph, "base", std::size(basePipelines), basePipelines, &baseMaterial));

        gerium_shader_t fullscreenShaders[2];
        fullscreenShaders[0].type = GERIUM_SHADER_TYPE_VERTEX;
        fullscreenShaders[0].name = "fullscreen.vert.glsl";
        fullscreenShaders[0].data = "#version 450\n"
                                    "\n"
                                    "layout(location = 0) out vec2 vTexCoord;"
                                    "\n"
                                    "void main() {\n"
                                    "    vTexCoord.xy = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);\n"
                                    "    gl_Position = vec4(vTexCoord.xy * 2.0f - 1.0f, 0.0f, 1.0f);\n"
                                    "    gl_Position.y = -gl_Position.y;\n"
                                    "}\n";
        fullscreenShaders[0].size = strlen(fullscreenShaders[0].data);

        fullscreenShaders[1].type = GERIUM_SHADER_TYPE_FRAGMENT;
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
        fullscreenShaders[1].size = strlen(fullscreenShaders[1].data);

        gerium_pipeline_t fullscreenPipelines[1]{};
        fullscreenPipelines[0].render_pass  = "present_pass";
        fullscreenPipelines[0].shader_count = std::size(fullscreenShaders);
        fullscreenPipelines[0].shaders      = fullscreenShaders;

        check(gerium_renderer_create_material(renderer,
                                              frameGraph,
                                              "fullscreen",
                                              std::size(fullscreenPipelines),
                                              fullscreenPipelines,
                                              &fullscreenMaterial));

        gerium_renderer_bind_resource(renderer, descriptorSet1, 0, frameGraph, "color");

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
        gerium_renderer_destroy_material(renderer, fullscreenMaterial);
        gerium_renderer_destroy_material(renderer, baseMaterial);
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

    gerium_renderer_render(renderer, frameGraph);
    gerium_renderer_present(renderer);

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

int main() {
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
