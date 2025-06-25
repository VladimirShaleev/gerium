/**
 * @file      gerium-structs.h
 * @brief     Data structures.
 * @details   Common structures used throughout the system.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @copyright MIT License
 */
#ifndef GERIUM_STRUCTS_H
#define GERIUM_STRUCTS_H

#include "gerium-callbacks.h"

GERIUM_BEGIN

/**
 * @brief   Keyboard input event.
 * @details Contains detailed information about keyboard state changes.
 */
typedef struct
{
    gerium_scancode_t      scancode; /**< Physical key position identifier (layout-independent). */
    gerium_key_code_t      code; /**< Logical key code (layout-dependent character mapping). */
    gerium_key_state_t     state; /**< Key press/release transition (Pressed/Released). */
    gerium_key_mod_flags_t modifiers; /**< Bitmask of active modifier keys (Shift/Ctrl/Alt etc.). */
    gerium_char_t          symbol[5]; /**< UTF-8 encoded character representation. */
} gerium_keyboard_event_t;

/**
 * @brief   Mouse input event.
 * @details Tracks mouse movement and button states.
 */
typedef struct
{
    gerium_uint32_t             id; /**< Input device identifier (for multi-pointer systems). */
    gerium_mouse_button_flags_t buttons; /**< Bitmask of pressed/released mouse buttons. */
    gerium_sint16_t             absolute_x; /**< Current absolute X position in window coordinates. */
    gerium_sint16_t             absolute_y; /**< Current absolute Y position in window coordinates. */
    gerium_sint16_t             delta_x; /**< Relative X movement since last event (taken from OS). */
    gerium_sint16_t             delta_y; /**< Relative Y movement since last event (taken from OS). */
    gerium_sint16_t             raw_delta_x; /**< Relative X movement since last event (taken from device, may be the same as gerium_mouse_event_t::delta_x on some platforms). */
    gerium_sint16_t             raw_delta_y; /**< Relative Y movement since last event (taken from device, may be the same as gerium_mouse_event_t::delta_y on some platforms). */
    gerium_float32_t            wheel_vertical; /**< Vertical scroll wheel delta. */
    gerium_float32_t            wheel_horizontal; /**< Horizontal scroll wheel delta. */
} gerium_mouse_event_t;

/**
 * @brief   Input event.
 * @details Unified input event structure with timestamp
 */
typedef struct
{
    gerium_event_type_t     type; /**< Event category (Keyboard/Mouse etc.). */
    gerium_uint64_t         timestamp; /**< Monotonic clock time in nanoseconds. */
    gerium_keyboard_event_t keyboard; /**< Keyboard-specific data. */
    gerium_mouse_event_t    mouse; /**< Mouse-specific data. */
} gerium_event_t;

/**
 * @brief   Display mode configuration.
 * @details Contains resolution and refresh rate settings.
 */
typedef struct
{
    gerium_uint16_t width; /**< Horizontal resolution in pixels. */
    gerium_uint16_t height; /**< Vertical resolution in pixels. */
    gerium_uint16_t refresh_rate; /**< Refresh rate in Hz (may be 0). */
} gerium_display_mode_t;

/**
 * @brief   Display device information.
 * @details Describes connected display properties and supported modes.
 */
typedef struct
{
    gerium_uint32_t              id; /**< Unique display identifier */
    gerium_utf8_t                name; /**< User-friendly display name (may be "Unknown") */
    gerium_utf8_t                gpu_name; /**< Associated GPU/adapter name (may be "Unknown") */
    gerium_utf8_t                device_name; /**< Hardware device identifier string (may be "Unknown") */
    gerium_uint32_t              mode_count; /**< Number of supported display modes. */
    const gerium_display_mode_t* modes; /**< Array of supported display modes. */
} gerium_display_info_t;

/**
 * @brief   Renderer configuration options.
 * @details Parameters controlling renderer initialization and behavior.
 */
typedef struct
{
    gerium_bool_t   debug_mode; /**< Enables validation layers, debug logs and debug utilities when *TRUE*. */
    gerium_uint32_t app_version; /**< User application version number. */
    gerium_uint32_t command_buffers_per_frame; /**< Number of command buffers allocated per frame (if 0, then the default value is 10). */
    gerium_uint32_t descriptor_sets_pool_size; /**< Number of descriptor sets per pool (if 0, then the default value is 4096). */
    gerium_uint32_t descriptor_pool_elements; /**< Size of descriptor pool element arrays (if 0, then the default value is 4096). */
    gerium_uint32_t dynamic_ubo_size; /**< Size of dynamic uniform buffer allocation (if 0, then the default value is 64 Kb). */
    gerium_uint32_t dynamic_ssbo_size; /**< Size of dynamic storage buffer allocation (if 0, then the default value is 256 Mb). */
} gerium_renderer_options_t;

/**
 * @brief   Texture specification.
 * @details Parameters defining texture resource properties.
 */
typedef struct
{
    gerium_uint16_t       width; /**< Texture width. */
    gerium_uint16_t       height; /**< Texture height. */
    gerium_uint16_t       depth; /**< Texture depth. */
    gerium_uint16_t       mipmaps; /**< Number of mipmap levels. */
    gerium_uint16_t       layers; /**< Array layer count. */
    gerium_format_t       format; /**< Texture format. */
    gerium_texture_type_t type; /**< Texture dimensionality. */
    gerium_utf8_t         name; /**< Debug name. */
} gerium_texture_info_t;

/**
 * @brief   GPU timing measurement.
 * @details Contains profiling data for GPU execution intervals.
 */
typedef struct
{
    gerium_utf8_t    name; /**< Name of the render pass. */
    gerium_float64_t elapsed; /**< Duration in milliseconds. */
    gerium_uint32_t  frame; /**< Frame number when measurement was taken. */
    gerium_uint32_t  depth; /**< Nesting level. */
} gerium_gpu_timestamp_t;

/**
 * @brief   Render pass definition.
 * @details Contains callbacks for different stages of pass execution.
 */
typedef struct
{
    gerium_frame_graph_prepare_callback_t prepare; /**< Preparing a frame before rendering (mey be null). */
    gerium_frame_graph_resize_callback_t  resize; /**< Resolution change handler, called when swapchain resizes (mey be null). */
    gerium_frame_graph_render_callback_t  render; /**< Rendering Pass. */
} gerium_render_pass_t;

/**
 * @brief   Rasterization state.
 * @details Controls polygon rendering behavior.
 */
typedef struct
{
    gerium_polygon_mode_t       polygon_mode; /**< Polygon rendering mode. */
    gerium_primitive_topology_t primitive_topology; /**< Primitive assembly type. */
    gerium_cull_mode_t          cull_mode; /**< Face culling behavior. */
    gerium_front_face_t         front_face; /**< Front face winding order. */
    gerium_bool_t               depth_clamp_enable; /**< Enables depth value clamping to [0,1] range. */
    gerium_bool_t               depth_bias_enable; /**< Enables depth bias. */
    gerium_float32_t            depth_bias_constant_factor; /**< Constant depth bias factor. */
    gerium_float32_t            depth_bias_clamp; /**< Maximum depth bias value. */
    gerium_float32_t            depth_bias_slope_factor; /**< Slope-scaled depth bias factor. */
    gerium_float32_t            line_width; /**< Rasterized line width. */
} gerium_rasterization_state_t;

/**
 * @brief   Stencil operation state.
 * @details Controls stencil test behavior for a single face.
 */
typedef struct
{
    gerium_stencil_op_t fail_op; /**< Action when stencil test fails. */
    gerium_stencil_op_t pass_op; /**< Action when both stencil and depth tests pass. */
    gerium_stencil_op_t depth_fail_op; /**< Action when stencil passes but depth fails. */
    gerium_compare_op_t compare_op; /**< Comparison operator for stencil test. */
    gerium_uint32_t     compare_mask; /**< Bitmask for stencil reference value comparison. */
    gerium_uint32_t     write_mask; /**< Bitmask controlling which stencil bits get written. */
    gerium_uint32_t     reference; /**< Reference value used for stencil comparisons. */
} gerium_stencil_op_state_t;

/**
 * @brief   Depth/stencil state.
 * @details Controls depth and stencil testing behavior.
 */
typedef struct
{
    gerium_bool_t             depth_test_enable; /**< Enables depth buffer testing. */
    gerium_bool_t             depth_write_enable; /**< Enables depth buffer writes. */
    gerium_bool_t             depth_bounds_test_enable; /**< Activates depth bounds testing. */
    gerium_bool_t             stencil_test_enable; /**< Enables stencil testing operations. */
    gerium_compare_op_t       depth_compare_op; /**< Depth comparison function. */
    gerium_stencil_op_state_t front; /**< Stencil operations for front-facing polygons. */
    gerium_stencil_op_state_t back; /**< Stencil operations for back-facing polygons. */
    gerium_float32_t          min_depth_bounds; /**< Minimum depth bound for depth test. */
    gerium_float32_t          max_depth_bounds; /**< Maximum depth bound for depth test. */
} gerium_depth_stencil_state_t;

/**
 * @brief   Color blending state.
 * @details Color blending configuration.
 */
typedef struct
{
    gerium_bool_t     logic_op_enable; /**< Enable logical operations. */
    gerium_logic_op_t logic_op; /**< Logical operation to apply. */
    gerium_float32_t  blend_constants[4]; /**< RGBA blend constants. */
} gerium_color_blend_state_t;

/**
 * @brief   Color blend attachment state.
 * @details Specifying a pipeline color blend attachment state.
 */
typedef struct
{
    gerium_bool_t         blend_enable; /**< Controls whether blending is enabled for the corresponding color attachment; if blending is not enabled, the source fragment’s color for that attachment is passed through unmodified. */
    gerium_blend_factor_t src_color_blend_factor; /**< Source color blend factor. */
    gerium_blend_factor_t dst_color_blend_factor; /**< Destination color blend factor. */
    gerium_blend_op_t     color_blend_op; /**< Selects which blend operation is used to calculate the RGB values to write to the color attachment. */
    gerium_blend_factor_t src_alpha_blend_factor; /**< Source alpha blend factor. */
    gerium_blend_factor_t dst_alpha_blend_factor; /**< Destination alpha blend factor. */
    gerium_blend_op_t     alpha_blend_op; /**< Selects which blend operation is used to calculate the alpha values to write to the color attachment. */
} gerium_color_blend_attachment_state_t;

/**
 * @brief   Color clear values.
 * @details Specifies RGBA values for color attachment clearing.
 */
typedef struct
{
    gerium_float32_t red; /**< Red channel clear value. */
    gerium_float32_t green; /**< Green channel clear value. */
    gerium_float32_t blue; /**< Blue channel clear value. */
    gerium_float32_t alpha; /**< Alpha channel clear value. */
} gerium_clear_color_attachment_state_t;

/**
 * @brief   Depth/stencil clear values.
 * @details Specifies values for depth/stencil attachment clearing.
 */
typedef struct
{
    gerium_float32_t depth; /**< Depth buffer clear value. */
    gerium_uint32_t  value; /**< Stencil buffer clear value. */
} gerium_clear_depth_stencil_attachment_state_t;

/**
 * @brief   Vertex attribute description.
 * @details Defines vertex input attribute description.
 * @sa      gerium_vertex_binding_t
 */
typedef struct
{
    gerium_uint16_t location; /**< Shader input location number for this attribute. */
    gerium_uint16_t binding; /**< Binding number which this attribute takes its data from. */
    gerium_uint16_t offset; /**< Byte offset of this attribute relative to the start of an element in the vertex input binding. */
    gerium_format_t format; /**< Size and type of the vertex attribute data. */
} gerium_vertex_attribute_t;

/**
 * @brief   Vertex binding description.
 * @details Defines vertex input binding description.
 * @sa      gerium_vertex_attribute_t
 */
typedef struct
{
    gerium_uint16_t      binding; /**< Binding number that this structure describes. */
    gerium_uint16_t      stride; /**< Byte stride between consecutive elements within the buffer. */
    gerium_vertex_rate_t input_rate; /**< Specifying whether vertex attribute addressing is a function of the vertex index or of the instance index. */
} gerium_vertex_binding_t;

/**
 * @brief   Shader macro definition.
 * @details Preprocessor macro for shader compilation.
 */
typedef struct
{
    gerium_char_t name[128]; /**< Macro identifier. */
    gerium_char_t value[128]; /**< Macro substitution value. */
} gerium_macro_definition_t;

/**
 * @brief   Shader program.
 * @details Contains shader code and compilation parameters.
 */
typedef struct
{
    gerium_shader_type_t             type; /**< Shader stage. */
    gerium_shader_language_t         lang; /**< Source language. */
    gerium_utf8_t                    name; /**< Debug name. */
    gerium_utf8_t                    entry_point; /**< Main function name (considered "main" if null). */
    gerium_cdata_t                   data; /**< Pointer to shader source. */
    gerium_uint32_t                  size; /**< Shader size in bytes. */
    gerium_uint32_t                  macro_count; /**< Number of preprocessor macros. */
    const gerium_macro_definition_t* macros; /**< Macro definitions array. */
} gerium_shader_t;

/**
 * @brief   Resource input.
 * @details Describes an input resource for a render pass.
 */
typedef struct
{
    gerium_resource_type_t type; /**< Resource type. */
    gerium_utf8_t          name; /**< Resource name. */
    gerium_bool_t          previous_frame; /**< Whether to use a resource from the previous frame (for example, when a reprojection of the previous frame is needed to render the current frame, as in TAA). */
} gerium_resource_input_t;

/**
 * @brief   Resource output.
 * @details Describes the output resource of a rendering pass.
 */
typedef struct
{
    gerium_resource_type_t                        type; /**< Resource type. */
    gerium_utf8_t                                 name; /**< Resource name. */
    gerium_bool_t                                 external; /**< The resource is external and is not managed by the frame graph (the frame graph, unlike external resources, automatically manages the life of the resource). */
    gerium_format_t                               format; /**< Resource format (used only if the resource type is not ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_uint16_t                               width; /**< Resource width, if 0, then the value is taken from the swapchain size (used only if the resource type is not ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_uint16_t                               height; /**< Resource height, if 0, then the value is taken from the swapchain size (used only if the resource type is not ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_uint16_t                               depth; /**< Resource height, should be set to 1 if it is not a 3D texture (used only if the resource type is not ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_uint16_t                               layers; /**< Number of layers, should be set to 1 if multiple layers are not needed (used only if the resource type is not ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_float32_t                              auto_scale; /**< Attachment scaling factor during initialization resource and resizing window; for example,
 * a value of 1.0 means that the size will always match the size of the swapchain, and a value
 * of 0.5 will correspond to half the size of the swapchain, regardless of how the swapchain
 * is resized; value 0 disables auto-resizing, and the attachment will not resize when the
 * swapchain size changes (used only if the resource type is not ::GERIUM_RESOURCE_TYPE_BUFFER)
 * (used only if the resource type is not ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_render_pass_op_t                       render_pass_op; /**< Operation performed during the rendering pass (used only if the resource type is not ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_color_component_flags_t                color_write_mask; /**< Specifying which of the R, G, B, and/or A components are enabled for writing (used only if the resource type is not ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_color_blend_attachment_state_t         color_blend_attachment; /**< Color blend attachment state (used only if the resource type is not ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_clear_color_attachment_state_t         clear_color_attachment; /**< Clear color target, only used if gerium_resource_output_t::render_pass_op is set to ::GERIUM_RENDER_PASS_OP_CLEAR (used only if the resource type is not ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_clear_depth_stencil_attachment_state_t clear_depth_stencil_attachment; /**< Clear depth/stencil target, only used if gerium_resource_output_t::render_pass_op is set to ::GERIUM_RENDER_PASS_OP_CLEAR (used only if the resource type is not ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_uint32_t                               size; /**< Buffer size in bytes (used only if the resource type is ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_uint32_t                               fill_value; /**< The value that the buffer will be filled with when allocated (used only if the resource type is ::GERIUM_RESOURCE_TYPE_BUFFER). */
    gerium_buffer_usage_flags_t                   usage; /**< Buffer usage flags (used only if the resource type is ::GERIUM_RESOURCE_TYPE_BUFFER). */
} gerium_resource_output_t;

/**
 * @brief   Pipeline configuration.
 * @details Complete specification of rendering pipeline state.
 */
typedef struct
{
    gerium_utf8_t                       render_pass; /**< Target render pass name. */
    const gerium_rasterization_state_t* rasterization; /**< Polygon rasterization configuration. */
    const gerium_depth_stencil_state_t* depth_stencil; /**< Depth/stencil test configuration. */
    const gerium_color_blend_state_t*   color_blend; /**< Color blending configuration. */
    gerium_uint32_t                     vertex_attribute_count; /**< Number of vertex input attributes. */
    const gerium_vertex_attribute_t*    vertex_attributes; /**< Vertex attribute descriptions. */
    gerium_uint32_t                     vertex_binding_count; /**< Number of vertex buffer bindings. */
    const gerium_vertex_binding_t*      vertex_bindings; /**< Vertex buffer binding descriptions. */
    gerium_uint32_t                     shader_count; /**< Number of shader stages. */
    const gerium_shader_t*              shaders; /**< Shader programs for each pipeline stage. */
} gerium_pipeline_t;

GERIUM_END

#endif /* GERIUM_STRUCTS_H */
