#ifndef CONVERTERS_HPP
#define CONVERTERS_HPP

#include "Common.hpp"

namespace YAML {

static gerium_uint8_t buffer[2048]{};
static gerium_uint32_t offset{};

void resetBuffer() noexcept {
    offset = 0;
}

template <typename T>
T* allocate(const T& data) {
    if (offset + sizeof(T) > std::size(buffer)) {
        std::bad_alloc();
    }
    auto result = new (buffer + offset) T();
    *result     = data;
    offset += sizeof(T);
    return result;
}

template <typename T>
T* allocate(const std::vector<T>& data) {
    if (offset + sizeof(T) * data.size() > std::size(buffer)) {
        std::bad_alloc();
    }
    auto results = new (buffer + offset) T[data.size()];
    for (int i = 0; i < data.size(); ++i) {
        results[i] = data[i];
    }
    offset += sizeof(T) * data.size();
    return results;
}

gerium_utf8_t allocate(const std::string& str) {
    if (str.length() == 0) {
        return nullptr;
    }
    if (offset + str.length() + 1 > std::size(buffer)) {
        std::bad_alloc();
    }
    auto result = new (buffer + offset) char[str.length() + 1]{};
    strncpy(result, str.c_str(), str.length());
    offset += str.length() + 1;
    return result;
}

template <typename E>
bool decodeEnum(const Node& node, E& rhs) {
    static_assert(std::is_enum_v<E>, "E is not enum");
    auto result = magic_enum::enum_cast<E>(node.as<std::string>());
    if (result.has_value()) {
        rhs = result.value();
        return true;
    }
    throw YAML::Exception(node.Mark(), YAML::ErrorMsg::INVALID_NODE);
}

template <>
struct convert<gerium_polygon_mode_t> {
    static Node encode(const gerium_polygon_mode_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_polygon_mode_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_primitive_topology_t> {
    static Node encode(const gerium_primitive_topology_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_primitive_topology_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_cull_mode_t> {
    static Node encode(const gerium_cull_mode_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_cull_mode_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_front_face_t> {
    static Node encode(const gerium_front_face_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_front_face_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_stencil_op_t> {
    static Node encode(const gerium_stencil_op_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_stencil_op_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_compare_op_t> {
    static Node encode(const gerium_compare_op_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_compare_op_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_logic_op_t> {
    static Node encode(const gerium_logic_op_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_logic_op_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_format_t> {
    static Node encode(const gerium_format_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_format_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_vertex_rate_t> {
    static Node encode(const gerium_vertex_rate_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_vertex_rate_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_shader_type_t> {
    static Node encode(const gerium_shader_type_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_shader_type_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_shader_languge_t> {
    static Node encode(const gerium_shader_languge_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_shader_languge_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_resource_type_t> {
    static Node encode(const gerium_resource_type_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_resource_type_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_render_pass_op_t> {
    static Node encode(const gerium_render_pass_op_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_render_pass_op_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_color_component_flags_t> {
    static Node encode(const gerium_color_component_flags_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_color_component_flags_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_blend_factor_t> {
    static Node encode(const gerium_blend_factor_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_blend_factor_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_blend_op_t> {
    static Node encode(const gerium_blend_op_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_blend_op_t& rhs) {
        return decodeEnum(node, rhs);
    }
};

template <>
struct convert<gerium_rasterization_state_t> {
    static Node encode(const gerium_rasterization_state_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_rasterization_state_t& rhs) {
        rhs.polygon_mode = node["polygon mode"].as<gerium_polygon_mode_t>(GERIUM_POLYGON_MODE_FILL);
        rhs.primitive_topology =
            node["primitive topology"].as<gerium_primitive_topology_t>(GERIUM_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        rhs.cull_mode          = node["cull mode"].as<gerium_cull_mode_t>(GERIUM_CULL_MODE_NONE);
        rhs.front_face         = node["front face"].as<gerium_front_face_t>(GERIUM_FRONT_FACE_COUNTER_CLOCKWISE);
        rhs.depth_clamp_enable = node["depth clamp enable"].as<bool>(false);
        rhs.depth_bias_enable  = node["depth bias enable"].as<bool>(false);
        rhs.depth_bias_constant_factor = node["depth bias constant factor"].as<float>(0.0f);
        rhs.depth_bias_clamp           = node["depth bias clamp"].as<float>(0.0f);
        rhs.depth_bias_slope_factor    = node["depth bias slope factor"].as<float>(0.0f);
        rhs.line_width                 = node["line width"].as<float>(1.0f);
        return true;
    }
};

template <>
struct convert<gerium_stencil_op_state_t> {
    static Node encode(const gerium_stencil_op_state_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_stencil_op_state_t& rhs) {
        rhs.fail_op       = node["fail op"].as<gerium_stencil_op_t>(GERIUM_STENCIL_OP_KEEP);
        rhs.pass_op       = node["pass op"].as<gerium_stencil_op_t>(GERIUM_STENCIL_OP_KEEP);
        rhs.depth_fail_op = node["depth fail op"].as<gerium_stencil_op_t>(GERIUM_STENCIL_OP_KEEP);
        rhs.compare_op    = node["compare op"].as<gerium_compare_op_t>(GERIUM_COMPARE_OP_NEVER);
        rhs.compare_mask  = node["compare mask"].as<gerium_uint32_t>(0);
        rhs.write_mask    = node["write mask"].as<gerium_uint32_t>(0);
        rhs.reference     = node["reference"].as<gerium_uint32_t>(0);
        return true;
    }
};

template <>
struct convert<gerium_depth_stencil_state_t> {
    static Node encode(const gerium_depth_stencil_state_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_depth_stencil_state_t& rhs) {
        rhs.depth_test_enable        = node["depth test enable"].as<bool>(false);
        rhs.depth_write_enable       = node["depth write enable"].as<bool>(false);
        rhs.depth_bounds_test_enable = node["depth bounds test enable"].as<bool>(false);
        rhs.stencil_test_enable      = node["stencil test enable"].as<bool>(false);
        rhs.depth_compare_op         = node["depth compare op"].as<gerium_compare_op_t>(GERIUM_COMPARE_OP_NEVER);
        rhs.front                    = node["front"].as<gerium_stencil_op_state_t>(gerium_stencil_op_state_t{});
        rhs.back                     = node["back"].as<gerium_stencil_op_state_t>(gerium_stencil_op_state_t{});
        rhs.min_depth_bounds         = node["min depth bounds"].as<float>(0.0f);
        rhs.max_depth_bounds         = node["max depth bounds"].as<float>(0.0f);
        return true;
    }
};

template <>
struct convert<gerium_color_blend_state_t> {
    static Node encode(const gerium_color_blend_state_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_color_blend_state_t& rhs) {
        const auto blendConstants =
            node["blend constants"].as<std::vector<float>>(std::vector{ 0.0f, 0.0f, 0.0f, 0.0f });

        rhs.logic_op_enable    = node["logic op enable"].as<bool>(false);
        rhs.logic_op           = node["logic op"].as<gerium_logic_op_t>(GERIUM_LOGIC_OP_CLEAR);
        rhs.blend_constants[0] = 0.0f;
        rhs.blend_constants[1] = 0.0f;
        rhs.blend_constants[2] = 0.0f;
        rhs.blend_constants[3] = 0.0f;
        for (int i = 0; i < blendConstants.size() && i < 4; ++i) {
            rhs.blend_constants[i] = blendConstants[i];
        }
        return true;
    }
};

template <>
struct convert<gerium_vertex_attribute_t> {
    static Node encode(const gerium_vertex_attribute_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_vertex_attribute_t& rhs) {
        rhs.location = node["location"].as<gerium_uint16_t>(0);
        rhs.binding  = node["binding"].as<gerium_uint16_t>(0);
        rhs.offset   = node["offset"].as<gerium_uint32_t>(0);
        rhs.format   = node["format"].as<gerium_format_t>(GERIUM_FORMAT_R8_UNORM);
        return true;
    }
};

template <>
struct convert<gerium_vertex_binding_t> {
    static Node encode(const gerium_vertex_binding_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_vertex_binding_t& rhs) {
        rhs.binding    = node["binding"].as<gerium_uint16_t>(0);
        rhs.stride     = node["stride"].as<gerium_uint16_t>(0);
        rhs.input_rate = node["input rate"].as<gerium_vertex_rate_t>(GERIUM_VERTEX_RATE_PER_VERTEX);
        return true;
    }
};

template <>
struct convert<gerium_macro_definition_t> {
    static Node encode(const gerium_macro_definition_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_macro_definition_t& rhs) {
        auto key   = node.begin()->first.as<std::string>();
        auto value = node.begin()->second.IsNull() ? "" : node.begin()->second.as<std::string>();
        strncpy(rhs.name, key.c_str(), std::size(rhs.name));
        strncpy(rhs.value, value.c_str(), std::size(rhs.value));
        if (key.length() == 0) {
            throw YAML::Exception(node.Mark(), YAML::ErrorMsg::INVALID_NODE);
        }
        return true;
    }
};

template <>
struct convert<gerium_shader_t> {
    static Node encode(const gerium_shader_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_shader_t& rhs) {
        const auto macros =
            node["macros"].as<std::vector<gerium_macro_definition_t>>(std::vector<gerium_macro_definition_t>{});

        rhs.type        = node["type"].as<gerium_shader_type_t>(GERIUM_SHADER_TYPE_VERTEX);
        rhs.lang        = node["lang"].as<gerium_shader_languge_t>(GERIUM_SHADER_LANGUAGE_UNKNOWN);
        rhs.name        = allocate(node["name"].as<std::string>());
        rhs.data        = allocate(node["data"].as<std::string>(""));
        rhs.size        = node["size"].as<gerium_uint32_t>(0);
        rhs.macro_count = (gerium_uint32_t) macros.size();
        rhs.macros      = allocate(macros);
        return true;
    }
};

template <>
struct convert<gerium_pipeline_t> {
    static Node encode(const gerium_pipeline_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_pipeline_t& rhs) {
        const auto vertexAttributes = node["vertex attributes"].as<std::vector<gerium_vertex_attribute_t>>(
            std::vector<gerium_vertex_attribute_t>{});
        const auto vertexBindings =
            node["vertex bindings"].as<std::vector<gerium_vertex_binding_t>>(std::vector<gerium_vertex_binding_t>{});
        const auto shaders = node["shaders"].as<std::vector<gerium_shader_t>>();
        gerium_rasterization_state_t defaultRasterization{};
        defaultRasterization.primitive_topology = GERIUM_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        defaultRasterization.line_width         = 1.0f;

        rhs.render_pass   = allocate(node["render pass"].as<std::string>());
        rhs.rasterization = allocate(node["rasterization"].as<gerium_rasterization_state_t>(defaultRasterization));
        rhs.depth_stencil =
            allocate(node["depth stencil"].as<gerium_depth_stencil_state_t>(gerium_depth_stencil_state_t{}));
        rhs.color_blend = allocate(node["color blend"].as<gerium_color_blend_state_t>(gerium_color_blend_state_t{}));
        rhs.vertex_attribute_count = (gerium_uint32_t) vertexAttributes.size();
        rhs.vertex_attributes      = allocate(vertexAttributes);
        rhs.vertex_binding_count   = (gerium_uint32_t) vertexBindings.size();
        rhs.vertex_bindings        = allocate(vertexBindings);
        rhs.shader_count           = (gerium_uint32_t) shaders.size();
        rhs.shaders                = allocate(shaders);
        return true;
    }
};

struct FrameGraphNode {
    std::string name;
    gerium_bool_t compute;
    std::vector<gerium_resource_input_t> inputs;
    std::vector<gerium_resource_output_t> outputs;
};

template <>
struct convert<gerium_color_blend_attachment_state_t> {
    static Node encode(const gerium_color_blend_attachment_state_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_color_blend_attachment_state_t& rhs) {
        rhs.blend_enable           = node["blend enable"].as<bool>(false);
        rhs.src_color_blend_factor = node["src color blend factor"].as<gerium_blend_factor_t>(GERIUM_BLEND_FACTOR_ZERO);
        rhs.dst_color_blend_factor = node["dst color blend factor"].as<gerium_blend_factor_t>(GERIUM_BLEND_FACTOR_ZERO);
        rhs.color_blend_op         = node["color blend op"].as<gerium_blend_op_t>(GERIUM_BLEND_OP_ADD);
        rhs.src_alpha_blend_factor = node["src alpha blend factor"].as<gerium_blend_factor_t>();
        rhs.dst_alpha_blend_factor = node["dst alpha blend factor"].as<gerium_blend_factor_t>();
        rhs.alpha_blend_op         = node["alpha blend op"].as<gerium_blend_op_t>(GERIUM_BLEND_OP_ADD);
        return true;
    }
};

template <>
struct convert<gerium_resource_input_t> {
    static Node encode(const gerium_resource_input_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_resource_input_t& rhs) {
        rhs.type           = node["type"].as<gerium_resource_type_t>();
        rhs.name           = allocate(node["name"].as<std::string>());
        rhs.previous_frame = node["previous frame"].as<bool>(false);
        return true;
    }
};

template <>
struct convert<gerium_resource_output_t> {
    static Node encode(const gerium_resource_output_t& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, gerium_resource_output_t& rhs) {
        const auto colorWriteMask = node["color write mask"].as<std::vector<gerium_color_component_flags_t>>(
            std::vector{ GERIUM_COLOR_COMPONENT_R_BIT,
                         GERIUM_COLOR_COMPONENT_G_BIT,
                         GERIUM_COLOR_COMPONENT_B_BIT,
                         GERIUM_COLOR_COMPONENT_A_BIT });
        auto clearColorAttachment =
            node["clear color attachment"].as<std::vector<float>>(std::vector{ 0.0f, 0.0f, 0.0f, 1.0f });
        auto clearDepthStencilAttachment = node["clear depth stencil attachment"];

        rhs.type             = node["type"].as<gerium_resource_type_t>();
        rhs.name             = allocate(node["name"].as<std::string>());
        rhs.external         = node["external"].as<bool>(false);
        rhs.format           = node["format"].as<gerium_format_t>();
        rhs.width            = node["width"].as<gerium_uint16_t>(0);
        rhs.height           = node["height"].as<gerium_uint16_t>(0);
        rhs.auto_scale       = node["auto scale"].as<float>(1.0f);
        rhs.render_pass_op   = node["render pass op"].as<gerium_render_pass_op_t>(GERIUM_RENDER_PASS_OP_DONT_CARE);
        rhs.color_write_mask = {};
        rhs.color_blend_attachment = node["color blend attachment"].as<gerium_color_blend_attachment_state_t>(
            gerium_color_blend_attachment_state_t{});

        for (auto mask : colorWriteMask) {
            rhs.color_write_mask |= mask;
        }

        rhs.clear_color_attachment.red   = clearColorAttachment.size() > 0 ? clearColorAttachment[0] : 0.0f;
        rhs.clear_color_attachment.green = clearColorAttachment.size() > 1 ? clearColorAttachment[1] : 0.0f;
        rhs.clear_color_attachment.blue  = clearColorAttachment.size() > 2 ? clearColorAttachment[2] : 0.0f;
        rhs.clear_color_attachment.alpha = clearColorAttachment.size() > 3 ? clearColorAttachment[3] : 1.0f;

        if (clearDepthStencilAttachment.IsDefined()) {
            if (!clearDepthStencilAttachment.IsSequence()) {
                throw YAML::Exception(node.Mark(), YAML::ErrorMsg::INVALID_NODE);
            }
            rhs.clear_depth_stencil_attachment.depth = clearDepthStencilAttachment[0].as<float>(1.0f);
            rhs.clear_depth_stencil_attachment.value = clearDepthStencilAttachment[1].as<gerium_uint32_t>(0);
        }
        return true;
    }
};

template <>
struct convert<FrameGraphNode> {
    static Node encode(const FrameGraphNode& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }

    static bool decode(const Node& node, FrameGraphNode& rhs) {
        rhs.name    = node["name"].as<std::string>();
        rhs.compute = node["compute"].as<bool>(false);
        rhs.inputs  = node["inputs"].as<decltype(rhs.inputs)>(decltype(rhs.inputs){});
        rhs.outputs = node["outputs"].as<decltype(rhs.outputs)>(decltype(rhs.outputs){});
        return true;
    }
};

} // namespace YAML

#endif
