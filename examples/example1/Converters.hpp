#ifndef CONVERTERS_HPP
#define CONVERTERS_HPP

#include "Common.hpp"

namespace YAML {

constexpr auto defaultFormat            = GERIUM_FORMAT_R8_UNORM;
constexpr auto defaultOp                = GERIUM_RENDER_PASS_OP_DONT_CARE;
constexpr auto defaultPolygonMode       = GERIUM_POLYGON_MODE_FILL;
constexpr auto defaultTopology          = GERIUM_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
constexpr auto defualtCullMode          = GERIUM_CULL_MODE_NONE;
constexpr auto defaultFrontFace         = GERIUM_FRONT_FACE_COUNTER_CLOCKWISE;
constexpr auto defaultLineWidth         = 1.0f;
constexpr auto defaultDepthTestEnable   = false;
constexpr auto defaultDepthWriteEnable  = false;
constexpr auto defaultStencilTestEnable = false;
constexpr auto defaultDepthCompareOp    = GERIUM_COMPARE_OP_NEVER;

static gerium_uint8_t buffer[8192]{};
static gerium_uint32_t offset{};

struct FrameGraphNode {
    std::string name;
    gerium_bool_t compute;
    std::vector<gerium_resource_input_t> inputs;
    std::vector<gerium_resource_output_t> outputs;
};

static struct Flags {
} flags{};

static struct Boolean {
} boolean{};

inline void resetBuffer() noexcept {
    offset = 0;
}

template <typename T>
T* allocate(size_t n = 1) {
    static_assert(std::is_trivial_v<T>);
    auto size = sizeof(T) * n;

    size = (size + 3) & ~3;
    if (offset + size >= std::size(buffer)) {
        std::bad_alloc();
    }
    auto results = new (buffer + offset) T[n]{};
    offset += size;
    return results;
}

template <typename T>
T* allocate(const T& data) {
    auto result = allocate<T>();

    *result = data;
    return result;
}

template <typename T>
T* allocate(const std::vector<T>& data) {
    if (data.empty()) {
        return nullptr;
    }
    auto results = allocate<T>(data.size());
    for (int i = 0; i < data.size(); ++i) {
        results[i] = data[i];
    }
    return results;
}

inline gerium_utf8_t allocate(const std::string& str) {
    if (str.length() == 0) {
        return nullptr;
    }
    auto result = allocate<char>(str.length() + 1);
    strncpy(result, str.c_str(), str.length());
    return result;
}

template <typename T>
struct encodeFail {
    static Node encode(const T& rhs) {
        throw YAML::Exception(YAML::Mark(), "Not implemented");
    }
};

template <typename E>
struct convertEnum : encodeFail<E> {
    static bool decode(const Node& node, E& rhs) {
        static_assert(std::is_enum_v<E>, "E is not enum");
        auto result = magic_enum::enum_cast<E>(node.as<std::string>());
        if (result.has_value()) {
            rhs = result.value();
            return true;
        }
        throw YAML::Exception(node.Mark(), YAML::ErrorMsg::INVALID_NODE);
    }
};

template <>
struct convert<gerium_polygon_mode_t> : convertEnum<gerium_polygon_mode_t> {};

template <>
struct convert<gerium_primitive_topology_t> : convertEnum<gerium_primitive_topology_t> {};

template <>
struct convert<gerium_cull_mode_t> : convertEnum<gerium_cull_mode_t> {};

template <>
struct convert<gerium_front_face_t> : convertEnum<gerium_front_face_t> {};

template <>
struct convert<gerium_stencil_op_t> : convertEnum<gerium_stencil_op_t> {};

template <>
struct convert<gerium_compare_op_t> : convertEnum<gerium_compare_op_t> {};

template <>
struct convert<gerium_logic_op_t> : convertEnum<gerium_logic_op_t> {};

template <>
struct convert<gerium_format_t> : convertEnum<gerium_format_t> {};

template <>
struct convert<gerium_vertex_rate_t> : convertEnum<gerium_vertex_rate_t> {};

template <>
struct convert<gerium_shader_type_t> : convertEnum<gerium_shader_type_t> {};

template <>
struct convert<gerium_shader_languge_t> : convertEnum<gerium_shader_languge_t> {};

template <>
struct convert<gerium_resource_type_t> : convertEnum<gerium_resource_type_t> {};

template <>
struct convert<gerium_render_pass_op_t> : convertEnum<gerium_render_pass_op_t> {};

template <>
struct convert<gerium_color_component_flags_t> : convertEnum<gerium_color_component_flags_t> {};

template <>
struct convert<gerium_blend_factor_t> : convertEnum<gerium_blend_factor_t> {};

template <>
struct convert<gerium_blend_op_t> : convertEnum<gerium_blend_op_t> {};

template <>
struct convert<gerium_buffer_usage_flags_t> : convertEnum<gerium_buffer_usage_flags_t> {};

template <typename T>
T createDefault() noexcept {
    return T{};
}

template <>
gerium_rasterization_state_t createDefault<gerium_rasterization_state_t>() noexcept {
    gerium_rasterization_state_t result{};
    result.polygon_mode       = defaultPolygonMode;
    result.primitive_topology = defaultTopology;
    result.cull_mode          = defualtCullMode;
    result.front_face         = defaultFrontFace;
    result.line_width         = defaultLineWidth;
    return result;
}

template <>
gerium_depth_stencil_state_t createDefault<gerium_depth_stencil_state_t>() noexcept {
    gerium_depth_stencil_state_t result{};
    result.depth_test_enable        = defaultDepthTestEnable;
    result.depth_write_enable       = defaultDepthWriteEnable;
    result.depth_bounds_test_enable = defaultStencilTestEnable;
    result.depth_compare_op         = defaultDepthCompareOp;
    result.front                    = createDefault<gerium_stencil_op_state_t>();
    result.back                     = createDefault<gerium_stencil_op_state_t>();
    return result;
}

template <>
gerium_vertex_attribute_t createDefault<gerium_vertex_attribute_t>() noexcept {
    gerium_vertex_attribute_t result{};
    result.format = GERIUM_FORMAT_R8_UNORM;
    return result;
}

template <>
gerium_clear_depth_stencil_attachment_state_t createDefault<gerium_clear_depth_stencil_attachment_state_t>() noexcept {
    gerium_clear_depth_stencil_attachment_state_t result{};
    result.depth = 1.0f;
    result.value = 0;
    return result;
}

template <>
gerium_resource_output_t createDefault<gerium_resource_output_t>() noexcept {
    constexpr auto writeMask = GERIUM_COLOR_COMPONENT_R_BIT | GERIUM_COLOR_COMPONENT_G_BIT |
                               GERIUM_COLOR_COMPONENT_B_BIT | GERIUM_COLOR_COMPONENT_A_BIT;

    gerium_resource_output_t result{};
    result.format                         = defaultFormat;
    result.width                          = 0;
    result.height                         = 0;
    result.depth                          = 1;
    result.layers                         = 1;
    result.auto_scale                     = 1.0f;
    result.render_pass_op                 = defaultOp;
    result.color_write_mask               = writeMask;
    result.clear_color_attachment         = { 0.0f, 0.0f, 0.0f, 1.0f };
    result.clear_depth_stencil_attachment = createDefault<gerium_clear_depth_stencil_attachment_state_t>();
    result.color_blend_attachment         = createDefault<gerium_color_blend_attachment_state_t>();
    return result;
}

// Deserialize a trivial structure
template <size_t N, typename T>
void read(const Node& node, const char (&key)[N], T& result) {
    result = node[key].template as<T>(result);
}

// Deserialize a trivial structure and store its address
template <size_t N, typename T>
void read(const Node& node, const char (&key)[N], T*& result) {
    using Type        = std::remove_cv_t<T>;
    auto defaultValue = createDefault<Type>();

    read(node, key, defaultValue);
    result = allocate(defaultValue);
}

// Deserialize string
template <size_t N>
void read(const Node& node, const char (&key)[N], gerium_utf8_t& result) {
    result = allocate(node[key].template as<std::string>(result ? result : ""));
}

// Deserialize byte array
template <size_t N>
void read(const Node& node, const char (&key)[N], gerium_cdata_t& result) {
    result = allocate(node[key].template as<std::string>(""));
}

// Deserialize a fixed size array
template <size_t N, typename T, size_t Size>
void read(const Node& node, const char (&key)[N], T (&results)[Size]) {
    std::vector<T> defaultValues(Size);
    for (size_t i = 0; i < Size; ++i) {
        defaultValues[i] = results[i];
    }
    const auto values = node[key].template as<std::vector<T>>(defaultValues);
    if (values.size() > 4) {
        throw YAML::Exception(node.Mark(), YAML::ErrorMsg::INVALID_NODE);
    }
    for (size_t i = 0; i < values.size(); ++i) {
        results[i] = values[i];
    }
}

// Deserialize the array and store the size and pointer
template <size_t N, typename Size, typename T>
void read(const Node& node, const char (&key)[N], Size& size, T*& results) {
    using Type  = std::remove_cv_t<T>;
    auto values = node[key].template as<std::vector<Type>>(std::vector<Type>{});
    size        = (Size) values.size();
    results     = allocate(values);
}

// Deserialize enum flags
template <size_t N, typename T>
void read(const Node& node, const char (&key)[N], T& result, Flags) {
    std::vector<T> values{};
    for (auto value : magic_enum::enum_values<T>()) {
        if (result & value) {
            values.push_back(value);
        }
    }
    values = node[key].template as<std::vector<T>>(values);
    result = {};
    for (auto value : values) {
        result |= value;
    }
}

// Deserialize boolean field
template <size_t N, typename T>
void read(const Node& node, const char (&key)[N], T& result, Boolean) {
    result = (T) node[key].template as<bool>(result);
}

template <>
struct convert<gerium_rasterization_state_t> : encodeFail<gerium_rasterization_state_t> {
    static bool decode(const Node& node, gerium_rasterization_state_t& rhs) {
        rhs = createDefault<std::remove_cvref_t<decltype(rhs)>>();
        read(node, "polygon mode", rhs.polygon_mode);
        read(node, "primitive topology", rhs.primitive_topology);
        read(node, "cull mode", rhs.cull_mode);
        read(node, "front face", rhs.front_face);
        read(node, "depth clamp enable", rhs.depth_clamp_enable, boolean);
        read(node, "depth bias enable", rhs.depth_bias_enable, boolean);
        read(node, "depth bias constant factor", rhs.depth_bias_constant_factor);
        read(node, "depth bias clamp", rhs.depth_bias_clamp);
        read(node, "depth bias slope factor", rhs.depth_bias_slope_factor);
        read(node, "line width", rhs.line_width);
        return true;
    }
};

template <>
struct convert<gerium_stencil_op_state_t> : encodeFail<gerium_stencil_op_state_t> {
    static bool decode(const Node& node, gerium_stencil_op_state_t& rhs) {
        rhs = createDefault<std::remove_cvref_t<decltype(rhs)>>();
        read(node, "fail op", rhs.fail_op);
        read(node, "pass op", rhs.pass_op);
        read(node, "depth fail op", rhs.depth_fail_op);
        read(node, "compare op", rhs.compare_op);
        read(node, "compare mask", rhs.compare_mask);
        read(node, "write mask", rhs.write_mask);
        read(node, "reference", rhs.reference);
        return true;
    }
};

template <>
struct convert<gerium_depth_stencil_state_t> : encodeFail<gerium_depth_stencil_state_t> {
    static bool decode(const Node& node, gerium_depth_stencil_state_t& rhs) {
        rhs = createDefault<std::remove_cvref_t<decltype(rhs)>>();
        read(node, "depth test enable", rhs.depth_test_enable, boolean);
        read(node, "depth write enable", rhs.depth_write_enable, boolean);
        read(node, "depth bounds test enable", rhs.depth_bounds_test_enable, boolean);
        read(node, "stencil test enable", rhs.stencil_test_enable, boolean);
        read(node, "depth compare op", rhs.depth_compare_op);
        read(node, "front", rhs.front);
        read(node, "back", rhs.back);
        read(node, "min depth bounds", rhs.min_depth_bounds);
        read(node, "max depth bounds", rhs.max_depth_bounds);
        return true;
    }
};

template <>
struct convert<gerium_color_blend_state_t> : encodeFail<gerium_color_blend_state_t> {
    static bool decode(const Node& node, gerium_color_blend_state_t& rhs) {
        rhs = createDefault<std::remove_cvref_t<decltype(rhs)>>();
        read(node, "logic op enable", rhs.logic_op_enable, boolean);
        read(node, "logic op", rhs.logic_op);
        read(node, "blend constants", rhs.blend_constants);
        return true;
    }
};

template <>
struct convert<gerium_vertex_attribute_t> : encodeFail<gerium_vertex_attribute_t> {
    static bool decode(const Node& node, gerium_vertex_attribute_t& rhs) {
        rhs = createDefault<std::remove_cvref_t<decltype(rhs)>>();
        read(node, "location", rhs.location);
        read(node, "binding", rhs.binding);
        read(node, "offset", rhs.offset);
        read(node, "format", rhs.format);
        return true;
    }
};

template <>
struct convert<gerium_vertex_binding_t> : encodeFail<gerium_vertex_binding_t> {
    static bool decode(const Node& node, gerium_vertex_binding_t& rhs) {
        rhs = createDefault<std::remove_cvref_t<decltype(rhs)>>();
        read(node, "binding", rhs.binding);
        read(node, "stride", rhs.stride);
        read(node, "input rate", rhs.input_rate);
        return true;
    }
};

template <>
struct convert<gerium_macro_definition_t> : encodeFail<gerium_macro_definition_t> {
    static bool decode(const Node& node, gerium_macro_definition_t& rhs) {
        rhs        = createDefault<std::remove_cvref_t<decltype(rhs)>>();
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
struct convert<gerium_shader_t> : encodeFail<gerium_shader_t> {
    static bool decode(const Node& node, gerium_shader_t& rhs) {
        rhs = createDefault<std::remove_cvref_t<decltype(rhs)>>();
        read(node, "type", rhs.type);
        read(node, "lang", rhs.lang);
        read(node, "name", rhs.name);
        read(node, "entry point", rhs.entry_point);
        read(node, "data", rhs.data);
        read(node, "size", rhs.size);
        read(node, "macros", rhs.macro_count, rhs.macros);
        return true;
    }
};

template <>
struct convert<gerium_pipeline_t> : encodeFail<gerium_pipeline_t> {
    static bool decode(const Node& node, gerium_pipeline_t& rhs) {
        rhs = createDefault<std::remove_cvref_t<decltype(rhs)>>();
        read(node, "render pass", rhs.render_pass);
        read(node, "rasterization", rhs.rasterization);
        read(node, "depth stencil", rhs.depth_stencil);
        read(node, "color blend", rhs.color_blend);
        read(node, "vertex attributes", rhs.vertex_attribute_count, rhs.vertex_attributes);
        read(node, "vertex bindings", rhs.vertex_binding_count, rhs.vertex_bindings);
        read(node, "shaders", rhs.shader_count, rhs.shaders);
        return true;
    }
};

template <>
struct convert<gerium_color_blend_attachment_state_t> : encodeFail<gerium_color_blend_attachment_state_t> {
    static bool decode(const Node& node, gerium_color_blend_attachment_state_t& rhs) {
        rhs = createDefault<std::remove_cvref_t<decltype(rhs)>>();
        read(node, "blend enable", rhs.blend_enable, boolean);
        read(node, "src color blend factor", rhs.src_color_blend_factor);
        read(node, "dst color blend factor", rhs.dst_color_blend_factor);
        read(node, "color blend op", rhs.color_blend_op);
        read(node, "src alpha blend factor", rhs.src_alpha_blend_factor);
        read(node, "dst alpha blend factor", rhs.dst_alpha_blend_factor);
        read(node, "alpha blend op", rhs.alpha_blend_op);
        return true;
    }
};

template <>
struct convert<gerium_resource_input_t> : encodeFail<gerium_resource_input_t> {
    static bool decode(const Node& node, gerium_resource_input_t& rhs) {
        rhs = createDefault<std::remove_cvref_t<decltype(rhs)>>();
        read(node, "type", rhs.type);
        read(node, "name", rhs.name);
        read(node, "previous frame", rhs.previous_frame, boolean);
        return true;
    }
};

template <>
struct convert<gerium_resource_output_t> : encodeFail<gerium_resource_output_t> {
    static bool decode(const Node& node, gerium_resource_output_t& rhs) {
        rhs = createDefault<std::remove_cvref_t<decltype(rhs)>>();
        gerium_float32_t colors[4]{ rhs.clear_color_attachment.red,
                                    rhs.clear_color_attachment.green,
                                    rhs.clear_color_attachment.blue,
                                    rhs.clear_color_attachment.alpha };
        read(node, "type", rhs.type);
        read(node, "name", rhs.name);
        read(node, "external", rhs.external, boolean);
        read(node, "format", rhs.format);
        read(node, "width", rhs.width);
        read(node, "height", rhs.height);
        read(node, "depth", rhs.depth);
        read(node, "layers", rhs.layers);
        read(node, "auto scale", rhs.auto_scale);
        read(node, "render pass op", rhs.render_pass_op);
        read(node, "color write mask", rhs.color_write_mask, flags);
        read(node, "color blend attachment", rhs.color_blend_attachment);
        read(node, "clear color attachment", colors);
        read(node, "size", rhs.size);
        read(node, "fill value", rhs.fill_value);
        read(node, "usage", rhs.usage, flags);

        rhs.clear_color_attachment.red   = colors[0];
        rhs.clear_color_attachment.green = colors[1];
        rhs.clear_color_attachment.blue  = colors[2];
        rhs.clear_color_attachment.alpha = colors[3];

        auto clearDepthStencilAttachment = node["clear depth stencil attachment"];
        if (clearDepthStencilAttachment.IsDefined()) {
            if (!clearDepthStencilAttachment.IsSequence()) {
                throw YAML::Exception(node.Mark(), YAML::ErrorMsg::INVALID_NODE);
            }
            rhs.clear_depth_stencil_attachment.depth =
                clearDepthStencilAttachment[0].as<float>(rhs.clear_depth_stencil_attachment.depth);
            rhs.clear_depth_stencil_attachment.value =
                clearDepthStencilAttachment[1].as<gerium_uint32_t>(rhs.clear_depth_stencil_attachment.value);
        }

        return true;
    }
};

template <>
struct convert<FrameGraphNode> : encodeFail<FrameGraphNode> {
    static bool decode(const Node& node, FrameGraphNode& rhs) {
        rhs = createDefault<std::remove_cvref_t<decltype(rhs)>>();
        read(node, "name", rhs.name);
        read(node, "compute", rhs.compute, boolean);
        read(node, "inputs", rhs.inputs);
        read(node, "outputs", rhs.outputs);
        return true;
    }
};

} // namespace YAML

#endif
