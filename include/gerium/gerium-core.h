/**
 * \file      gerium-core.h
 * \brief     gerium API Core
 * \author    Vladimir Shaleev
 * \copyright MIT License
 */

#ifndef GERIUM_CORE_H
#define GERIUM_CORE_H

#include "gerium-version.h"
#include "gerium-platform.h"

GERIUM_BEGIN

GERIUM_TYPE(gerium_logger)
GERIUM_TYPE(gerium_application)
GERIUM_TYPE(gerium_renderer)
GERIUM_TYPE(gerium_command_buffer)
GERIUM_TYPE(gerium_frame_graph)
GERIUM_TYPE(gerium_profiler)

GERIUM_HANDLE(gerium_buffer)
GERIUM_HANDLE(gerium_texture)
GERIUM_HANDLE(gerium_material)
GERIUM_HANDLE(gerium_descriptor_set)

typedef enum
{
    GERIUM_RESULT_SUCCESS                           = 0,
    GERIUM_RESULT_SKIP_FRAME                        = 1,
    GERIUM_RESULT_ERROR_UNKNOWN                     = 2,
    GERIUM_RESULT_ERROR_OUT_OF_MEMORY               = 3,
    GERIUM_RESULT_ERROR_NOT_IMPLEMENTED             = 4,
    GERIUM_RESULT_ERROR_FROM_CALLBACK               = 5,
    GERIUM_RESULT_ERROR_FEATURE_NOT_SUPPORTED       = 6,
    GERIUM_RESULT_ERROR_INVALID_ARGUMENT            = 7,
    GERIUM_RESULT_ERROR_NO_DISPLAY                  = 8,
    GERIUM_RESULT_ERROR_APPLICATION_ALREADY_RUNNING = 9,
    GERIUM_RESULT_ERROR_APPLICATION_NOT_RUNNING     = 10,
    GERIUM_RESULT_ERROR_APPLICATION_TERMINATED      = 11,
    GERIUM_RESULT_ERROR_CHANGE_DISPLAY_MODE         = 12,
    GERIUM_RESULT_MAX_ENUM                          = 0x7FFFFFFF
} gerium_result_t;

typedef enum {
    GERIUM_LOGGER_LEVEL_VERBOSE  = 0,
    GERIUM_LOGGER_LEVEL_DEBUG    = 1,
    GERIUM_LOGGER_LEVEL_INFO     = 2,
    GERIUM_LOGGER_LEVEL_WARNING  = 3,
    GERIUM_LOGGER_LEVEL_ERROR    = 4,
    GERIUM_LOGGER_LEVEL_FATAL    = 5,
    GERIUM_LOGGER_LEVEL_OFF      = 6,
    GERIUM_LOGGER_LEVEL_MAX_ENUM = 0x7FFFFFFF
} gerium_logger_level_t;

typedef enum
{
    GERIUM_RUNTIME_PLATFORM_UNKNOWN  = 0,
    GERIUM_RUNTIME_PLATFORM_ANDROID  = 1,
    GERIUM_RUNTIME_PLATFORM_IOS      = 2,
    GERIUM_RUNTIME_PLATFORM_WEB      = 3,
    GERIUM_RUNTIME_PLATFORM_WINDOWS  = 4,
    GERIUM_RUNTIME_PLATFORM_LINUX    = 5,
    GERIUM_RUNTIME_PLATFORM_MAC_OS   = 6,
    GERIUM_RUNTIME_PLATFORM_MAX_ENUM = 0x7FFFFFFF
} gerium_runtime_platform_t;

typedef enum
{
    GERIUM_APPLICATION_STATE_UNKNOWN       = 0,
    GERIUM_APPLICATION_STATE_CREATE        = 1,
    GERIUM_APPLICATION_STATE_DESTROY       = 2,
    GERIUM_APPLICATION_STATE_INITIALIZE    = 3,
    GERIUM_APPLICATION_STATE_UNINITIALIZE  = 4,
    GERIUM_APPLICATION_STATE_GOT_FOCUS     = 5,
    GERIUM_APPLICATION_STATE_LOST_FOCUS    = 6,
    GERIUM_APPLICATION_STATE_VISIBLE       = 7,
    GERIUM_APPLICATION_STATE_INVISIBLE     = 8,
    GERIUM_APPLICATION_STATE_NORMAL        = 9,
    GERIUM_APPLICATION_STATE_MINIMIZE      = 10,
    GERIUM_APPLICATION_STATE_MAXIMIZE      = 11,
    GERIUM_APPLICATION_STATE_FULLSCREEN    = 12,
    GERIUM_APPLICATION_STATE_RESIZE        = 13,
    GERIUM_APPLICATION_STATE_RESIZED       = 14,
    GERIUM_APPLICATION_STATE_MAX_ENUM      = 0x7FFFFFFF
} gerium_application_state_t;

typedef enum
{
    GERIUM_APPLICATION_STYLE_NONE_BIT        = 0,
    GERIUM_APPLICATION_STYLE_RESIZABLE_BIT   = 1,
    GERIUM_APPLICATION_STYLE_MINIMIZABLE_BIT = 2,
    GERIUM_APPLICATION_STYLE_MAXIMIZABLE_BIT = 4,
    GERIUM_APPLICATION_STYLE_MAX_ENUM        = 0x7FFFFFFF
} gerium_application_style_flags_t;
GERIUM_FLAGS(gerium_application_style_flags_t)

typedef enum
{
    GERIUM_FORMAT_R8_UNORM            = 0, 
    GERIUM_FORMAT_R8_SNORM            = 1, 
    GERIUM_FORMAT_R8_UINT             = 2, 
    GERIUM_FORMAT_R8_SINT             = 3, 
    GERIUM_FORMAT_R8G8_UNORM          = 4, 
    GERIUM_FORMAT_R8G8_SNORM          = 5, 
    GERIUM_FORMAT_R8G8_UINT           = 6, 
    GERIUM_FORMAT_R8G8_SINT           = 7, 
    GERIUM_FORMAT_R8G8B8_UNORM        = 8, 
    GERIUM_FORMAT_R8G8B8_SNORM        = 9, 
    GERIUM_FORMAT_R8G8B8_UINT         = 10,
    GERIUM_FORMAT_R8G8B8_SINT         = 11,
    GERIUM_FORMAT_R8G8B8_SRGB         = 12,
    GERIUM_FORMAT_R4G4B4A4_UNORM      = 13,
    GERIUM_FORMAT_R5G5B5A1_UNORM      = 14,
    GERIUM_FORMAT_R8G8B8A8_UNORM      = 15,
    GERIUM_FORMAT_R8G8B8A8_SNORM      = 16,
    GERIUM_FORMAT_R8G8B8A8_UINT       = 17,
    GERIUM_FORMAT_R8G8B8A8_SINT       = 18,
    GERIUM_FORMAT_R8G8B8A8_SRGB       = 19,
    GERIUM_FORMAT_A2R10G10B10_UNORM   = 20,
    GERIUM_FORMAT_A2R10G10B10_UINT    = 21,
    GERIUM_FORMAT_R16_UINT            = 22,
    GERIUM_FORMAT_R16_SINT            = 23,
    GERIUM_FORMAT_R16_SFLOAT          = 24,
    GERIUM_FORMAT_R16G16_UINT         = 25,
    GERIUM_FORMAT_R16G16_SINT         = 26,
    GERIUM_FORMAT_R16G16_SFLOAT       = 27,
    GERIUM_FORMAT_R16G16B16_UINT      = 28,
    GERIUM_FORMAT_R16G16B16_SINT      = 29,
    GERIUM_FORMAT_R16G16B16_SFLOAT    = 30,
    GERIUM_FORMAT_R16G16B16A16_UINT   = 31,
    GERIUM_FORMAT_R16G16B16A16_SINT   = 32,
    GERIUM_FORMAT_R16G16B16A16_SFLOAT = 33,
    GERIUM_FORMAT_R32_UINT            = 34,
    GERIUM_FORMAT_R32_SINT            = 35,
    GERIUM_FORMAT_R32_SFLOAT          = 36,
    GERIUM_FORMAT_R32G32_UINT         = 37,
    GERIUM_FORMAT_R32G32_SINT         = 38,
    GERIUM_FORMAT_R32G32_SFLOAT       = 39,
    GERIUM_FORMAT_R32G32B32_UINT      = 40,
    GERIUM_FORMAT_R32G32B32_SINT      = 41,
    GERIUM_FORMAT_R32G32B32_SFLOAT    = 42,
    GERIUM_FORMAT_R32G32B32A32_UINT   = 43,
    GERIUM_FORMAT_R32G32B32A32_SINT   = 44,
    GERIUM_FORMAT_R32G32B32A32_SFLOAT = 45,
    GERIUM_FORMAT_B10G11R11_UFLOAT    = 46,
    GERIUM_FORMAT_E5B9G9R9_UFLOAT     = 47,
    GERIUM_FORMAT_D16_UNORM           = 48,
    GERIUM_FORMAT_X8_D24_UNORM        = 49,
    GERIUM_FORMAT_D32_SFLOAT          = 50,
    GERIUM_FORMAT_S8_UINT             = 51,
    GERIUM_FORMAT_D24_UNORM_S8_UINT   = 52,
    GERIUM_FORMAT_D32_SFLOAT_S8_UINT  = 53,
    GERIUM_FORMAT_MAX_ENUM            = 0x7FFFFFFF
} gerium_format_t;

typedef enum
{
    GERIUM_POLYGON_MODE_FILL     = 0,
    GERIUM_POLYGON_MODE_LINE     = 1,
    GERIUM_POLYGON_MODE_POINT    = 2,
    GERIUM_POLYGON_MODE_MAX_ENUM = 0x7FFFFFFF
} gerium_polygon_mode_t;

typedef enum
{
    GERIUM_CULL_MODE_NONE           = 0,
    GERIUM_CULL_MODE_FRONT          = 1,
    GERIUM_CULL_MODE_BACK           = 2,
    GERIUM_CULL_MODE_FRONT_AND_BACK = 3,
    GERIUM_CULL_MODE_MAX_ENUM       = 0x7FFFFFFF
} gerium_cull_mode_t;

typedef enum
{
    GERIUM_FRONT_FACE_COUNTER_CLOCKWISE = 0,
    GERIUM_FRONT_FACE_CLOCKWISE         = 1,
    GERIUM_FRONT_FACE_MAX_ENUM          = 0x7FFFFFFF
} gerium_front_face_t;

typedef enum
{
    GERIUM_COMPARE_OP_NEVER            = 0,
    GERIUM_COMPARE_OP_ALWAYS           = 1,
    GERIUM_COMPARE_OP_LESS             = 2,
    GERIUM_COMPARE_OP_LESS_OR_EQUAL    = 3,
    GERIUM_COMPARE_OP_GREATER          = 4,
    GERIUM_COMPARE_OP_GREATER_OR_EQUAL = 5,
    GERIUM_COMPARE_OP_EQUAL            = 6,
    GERIUM_COMPARE_OP_NOT_EQUAL        = 7,
    GERIUM_COMPARE_OP_MAX_ENUM         = 0X7FFFFFFF
} gerium_compare_op_t;

typedef enum
{
    GERIUM_STENCIL_OP_KEEP                = 0,
    GERIUM_STENCIL_OP_ZERO                = 1,
    GERIUM_STENCIL_OP_REPLACE             = 2,
    GERIUM_STENCIL_OP_INVERT              = 3,
    GERIUM_STENCIL_OP_INCREMENT_AND_CLAMP = 4,
    GERIUM_STENCIL_OP_DECREMENT_AND_CLAMP = 5,
    GERIUM_STENCIL_OP_INCREMENT_AND_WRAP  = 6,
    GERIUM_STENCIL_OP_DECREMENT_AND_WRAP  = 7,
    GERIUM_STENCIL_OP_MAX_ENUM            = 0X7FFFFFFF
} gerium_stencil_op_t;

typedef enum
{
    GERIUM_LOGIC_OP_CLEAR         = 0,
    GERIUM_LOGIC_OP_SET           = 1,
    GERIUM_LOGIC_OP_NO_OP         = 2,
    GERIUM_LOGIC_OP_COPY          = 3,
    GERIUM_LOGIC_OP_COPY_INVERTED = 4,
    GERIUM_LOGIC_OP_AND           = 5,
    GERIUM_LOGIC_OP_AND_REVERSE   = 6,
    GERIUM_LOGIC_OP_AND_INVERTED  = 7,
    GERIUM_LOGIC_OP_NAND          = 8,
    GERIUM_LOGIC_OP_OR            = 9,
    GERIUM_LOGIC_OP_OR_REVERSE    = 10,
    GERIUM_LOGIC_OP_OR_INVERTED   = 11,
    GERIUM_LOGIC_OP_NOR           = 12,
    GERIUM_LOGIC_OP_XOR           = 13,
    GERIUM_LOGIC_OP_EQUIVALENT    = 14,
    GERIUM_LOGIC_OP_INVERT        = 15,
    GERIUM_LOGIC_OP_MAX_ENUM      = 0X7FFFFFFF
} gerium_logic_op_t;

typedef enum
{
    GERIUM_BLEND_OP_ADD              = 0,
    GERIUM_BLEND_OP_SUBTRACT         = 1,
    GERIUM_BLEND_OP_REVERSE_SUBTRACT = 2,
    GERIUM_BLEND_OP_MIN              = 3,
    GERIUM_BLEND_OP_MAX              = 4,
    GERIUM_BLEND_OP_MAX_ENUM         = 0X7FFFFFFF
} gerium_blend_op_t;

typedef enum
{
    GERIUM_RENDER_PASS_OP_DONT_CARE = 0,
    GERIUM_RENDER_PASS_OP_LOAD      = 1,
    GERIUM_RENDER_PASS_OP_CLEAR     = 2,
    GERIUM_RENDER_PASS_OP_MAX_ENUM  = 0x7FFFFFFF
} gerium_render_pass_op_t;

typedef enum
{
    GERIUM_BLEND_FACTOR_ZERO                     = 0,
    GERIUM_BLEND_FACTOR_ONE                      = 1,
    GERIUM_BLEND_FACTOR_SRC_COLOR                = 2,
    GERIUM_BLEND_FACTOR_SRC_ALPHA                = 3,
    GERIUM_BLEND_FACTOR_SRC_ALPHA_SATURATE       = 4,
    GERIUM_BLEND_FACTOR_DST_COLOR                = 5,
    GERIUM_BLEND_FACTOR_DST_ALPHA                = 6,
    GERIUM_BLEND_FACTOR_CONSTANT_COLOR           = 7,
    GERIUM_BLEND_FACTOR_CONSTANT_ALPHA           = 8,
    GERIUM_BLEND_FACTOR_SRC1_COLOR               = 9,
    GERIUM_BLEND_FACTOR_SRC1_ALPHA               = 10,
    GERIUM_BLEND_FACTOR_ONE_MINUS_SRC_COLOR      = 11,
    GERIUM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA      = 12,
    GERIUM_BLEND_FACTOR_ONE_MINUS_DST_COLOR      = 13,
    GERIUM_BLEND_FACTOR_ONE_MINUS_DST_ALPHA      = 14,
    GERIUM_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = 15,
    GERIUM_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = 16,
    GERIUM_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR     = 17,
    GERIUM_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA     = 18,
    GERIUM_BLEND_FACTOR_MAX_ENUM = 0X7FFFFFFF
} gerium_blend_factor_t;

typedef enum
{
    GERIUM_COLOR_COMPONENT_R_BIT    = 1,
    GERIUM_COLOR_COMPONENT_G_BIT    = 2,
    GERIUM_COLOR_COMPONENT_B_BIT    = 4,
    GERIUM_COLOR_COMPONENT_A_BIT    = 8,
    GERIUM_COLOR_COMPONENT_MAX_ENUM = 0X7FFFFFFF
} gerium_color_component_flags_t;
GERIUM_FLAGS(gerium_color_component_flags_t)

typedef enum
{
    GERIUM_BUFFER_USAGE_VERTEX_BIT  = 1,
    GERIUM_BUFFER_USAGE_INDEX_BIT   = 2,
    GERIUM_BUFFER_USAGE_UNIFORM_BIT = 4,
    GERIUM_BUFFER_USAGE_MAX_ENUM    = 0x7FFFFFFF
} gerium_buffer_usage_flags_t;
GERIUM_FLAGS(gerium_buffer_usage_flags_t)

typedef enum
{
    GERIUM_TEXTURE_TYPE_1D         = 0,
    GERIUM_TEXTURE_TYPE_2D         = 1,
    GERIUM_TEXTURE_TYPE_3D         = 2,
    GERIUM_TEXTURE_TYPE_1D_ARRAY   = 3,
    GERIUM_TEXTURE_TYPE_2D_ARRAY   = 4,
    GERIUM_TEXTURE_TYPE_CUBE_ARRAY = 5,
    GERIUM_TEXTURE_TYPE_MAX_ENUM   = 0x7FFFFFFF
} gerium_texture_type_t;

typedef enum
{
    GERIUM_RESOURCE_TYPE_BUFFER     = 0,
    GERIUM_RESOURCE_TYPE_TEXTURE    = 1,
    GERIUM_RESOURCE_TYPE_ATTACHMENT = 2,
    GERIUM_RESOURCE_TYPE_REFERENCE  = 3,
    GERIUM_RESOURCE_TYPE_MAX_ENUM   = 0x7FFFFFFF
} gerium_resource_type_t;

typedef enum {
    GERIUM_VERTEX_RATE_PER_VERTEX   = 0,
    GERIUM_VERTEX_RATE_PER_INSTANCE = 1,
    GERIUM_VERTEX_RATE_MAX_ENUM     = 0x7FFFFFFF
} gerium_vertex_rate_t;

typedef enum {
    GERIUM_SHADER_TYPE_VERTEX   = 0,
    GERIUM_SHADER_TYPE_FRAGMENT = 1,
    GERIUM_SHADER_TYPE_MAX_ENUM = 0x7FFFFFFF
} gerium_shader_type_t;

typedef gerium_bool_t
(*gerium_application_frame_func_t)(gerium_application_t application,
                                   gerium_data_t data,
                                   gerium_float32_t elapsed);

typedef gerium_bool_t
(*gerium_application_state_func_t)(gerium_application_t application,
                                   gerium_data_t data,
                                   gerium_application_state_t state);

typedef gerium_bool_t
(*gerium_frame_graph_prepare_func_t)(gerium_frame_graph_t frame_graph,
                                     gerium_renderer_t renderer,
                                     gerium_data_t data);

typedef gerium_bool_t
(*gerium_frame_graph_resize_func_t)(gerium_frame_graph_t frame_graph,
                                    gerium_renderer_t renderer,
                                    gerium_data_t data);

typedef gerium_bool_t
(*gerium_frame_graph_render_func_t)(gerium_frame_graph_t frame_graph,
                                    gerium_renderer_t renderer,
                                    gerium_command_buffer_t command_buffer,
                                    gerium_data_t data);

typedef struct
{
    gerium_uint16_t width;
    gerium_uint16_t height;
    gerium_uint16_t refresh_rate;
} gerium_display_mode_t;

typedef struct
{
    gerium_uint32_t              id;
    gerium_utf8_t                name;
    gerium_utf8_t                gpu_name;
    gerium_utf8_t                device_name;
    gerium_uint32_t              mode_count;
    const gerium_display_mode_t* modes;
} gerium_display_info_t;

typedef struct
{
    gerium_uint16_t       width;
    gerium_uint16_t       height;
    gerium_uint16_t       depth;
    gerium_uint16_t       mipmaps;
    gerium_format_t       format;
    gerium_texture_type_t type;
    gerium_cdata_t        data;
    gerium_utf8_t         name;
} gerium_texture_creation_t;

typedef struct
{
    gerium_utf8_t    name;
    gerium_float64_t elapsed;
    gerium_uint32_t  frame;
    gerium_uint32_t  depth;
} gerium_gpu_timestamp_t;

typedef struct
{
    gerium_frame_graph_prepare_func_t prepare;
    gerium_frame_graph_resize_func_t  resize;
    gerium_frame_graph_render_func_t  render;
} gerium_render_pass_t;

typedef struct
{
    gerium_polygon_mode_t polygon_mode;
    gerium_cull_mode_t    cull_mode;
    gerium_front_face_t   front_face;
    gerium_bool_t         depth_clamp_enable;
    gerium_bool_t         depth_bias_enable;
    gerium_float32_t      depth_bias_constant_factor;
    gerium_float32_t      depth_bias_clamp;
    gerium_float32_t      depth_bias_slope_factor;
    gerium_float32_t      line_width;
} gerium_rasterization_state_t;

typedef struct
{
    gerium_stencil_op_t fail_op;
    gerium_stencil_op_t pass_op;
    gerium_stencil_op_t depth_fail_op;
    gerium_compare_op_t compare_op;
    gerium_uint32_t     compare_mask;
    gerium_uint32_t     write_mask;
    gerium_uint32_t     reference;
} gerium_stencil_op_state_t;

typedef struct
{
    gerium_bool_t             depth_test_enable;
    gerium_bool_t             depth_write_enable;
    gerium_bool_t             depth_bounds_test_enable;
    gerium_bool_t             stencil_test_enable;
    gerium_compare_op_t       depth_compare_op;
    gerium_stencil_op_state_t front;
    gerium_stencil_op_state_t back;
    gerium_float32_t          min_depth_bounds;
    gerium_float32_t          max_depth_bounds;
} gerium_depth_stencil_state_t;

typedef struct
{
    gerium_bool_t     logic_op_enable;
    gerium_logic_op_t logic_op;
    gerium_float32_t  blend_constants[4];
} gerium_color_blend_state_t;

typedef struct
{
    gerium_bool_t         blend_enable;
    gerium_blend_factor_t src_color_blend_factor;
    gerium_blend_factor_t dst_color_blend_factor;
    gerium_blend_op_t     color_blend_op;
    gerium_blend_factor_t src_alpha_blend_factor;
    gerium_blend_factor_t dst_alpha_blend_factor;
    gerium_blend_op_t     alpha_blend_op;
} gerium_color_blend_attachment_state_t;

typedef struct
{
    gerium_float32_t red;
    gerium_float32_t green;
    gerium_float32_t blue;
    gerium_float32_t alpha;
} gerium_clear_color_attachment_state_t;

typedef struct
{
    gerium_float32_t depth;
    gerium_uint32_t  value;
} gerium_clear_depth_stencil_attachment_state_t;

typedef struct
{
    gerium_uint16_t location;
    gerium_uint16_t binding;
    gerium_uint32_t offset;
    gerium_format_t format;
} gerium_vertex_attribute_t;

typedef struct
{
    gerium_uint16_t             binding;
    gerium_uint16_t             stride;
    gerium_vertex_rate_t inputRate;
} gerium_vertex_binding_t;

typedef struct
{
    gerium_shader_type_t type;
    gerium_utf8_t        name;
    gerium_utf8_t        data;
    gerium_uint32_t      size;
} gerium_shader_t;

typedef struct
{
    gerium_resource_type_t         type;
    gerium_utf8_t                  name;
} gerium_resource_input_t;

typedef struct
{
    gerium_resource_type_t                        type;
    gerium_utf8_t                                 name;
    gerium_bool_t                                 external;
    gerium_format_t                               format;
    gerium_uint16_t                               width;
    gerium_uint16_t                               height;
    gerium_float32_t                              auto_scale;
    gerium_render_pass_op_t                       render_pass_op;
    gerium_color_component_flags_t                color_write_mask;
    gerium_color_blend_attachment_state_t         color_blend_attachment;
    gerium_clear_color_attachment_state_t         clear_color_attachment;
    gerium_clear_depth_stencil_attachment_state_t clear_depth_stencil_attachment;
} gerium_resource_output_t;

typedef struct
{
    gerium_utf8_t                       render_pass;
    const gerium_rasterization_state_t* rasterization;
    const gerium_depth_stencil_state_t* depth_stencil;
    const gerium_color_blend_state_t*   color_blend;
    gerium_uint32_t                     vertex_attribute_count;
    const gerium_vertex_attribute_t*    vertex_attributes;
    gerium_uint32_t                     vertex_binding_count;
    const gerium_vertex_binding_t*      vertex_bindings;
    gerium_uint32_t                     shader_count;
    const gerium_shader_t*              shaders;
} gerium_pipeline_t;

gerium_public gerium_uint32_t
gerium_version(void);

gerium_public gerium_utf8_t
gerium_version_string(void);

gerium_public gerium_utf8_t
gerium_result_to_string(gerium_result_t result);

gerium_public gerium_result_t
gerium_logger_create(gerium_utf8_t tag,
                     gerium_logger_t* logger);

gerium_public gerium_logger_t
gerium_logger_reference(gerium_logger_t logger);

gerium_public void
gerium_logger_destroy(gerium_logger_t logger);

gerium_public gerium_logger_level_t
gerium_logger_get_level(gerium_logger_t logger);

gerium_public void
gerium_logger_set_level(gerium_logger_t logger,
                        gerium_logger_level_t level);

gerium_public void
gerium_logger_set_level_by_tag(gerium_utf8_t tag,
                               gerium_logger_level_t level);

gerium_public void
gerium_logger_print(gerium_logger_t logger,
                    gerium_logger_level_t level,
                    gerium_utf8_t message);

gerium_public gerium_result_t
gerium_application_create(gerium_utf8_t title,
                          gerium_uint32_t width,
                          gerium_uint32_t height,
                          gerium_application_t* application);

gerium_public gerium_application_t
gerium_application_reference(gerium_application_t application);

gerium_public void
gerium_application_destroy(gerium_application_t application);

gerium_public gerium_runtime_platform_t
gerium_application_get_platform(gerium_application_t application);

gerium_public gerium_application_frame_func_t
gerium_application_get_frame_func(gerium_application_t application,
                                  gerium_data_t* data);

gerium_public void
gerium_application_set_frame_func(gerium_application_t application,
                                  gerium_application_frame_func_t callback,
                                  gerium_data_t data);

gerium_public gerium_application_state_func_t
gerium_application_get_state_func(gerium_application_t application,
                                  gerium_data_t* data);

gerium_public void
gerium_application_set_state_func(gerium_application_t application,
                                  gerium_application_state_func_t callback,
                                  gerium_data_t data);

gerium_public gerium_result_t
gerium_application_get_display_info(gerium_application_t application,
                                    gerium_uint32_t* display_count,
                                    gerium_display_info_t* displays);

gerium_public gerium_bool_t
gerium_application_is_fullscreen(gerium_application_t application);

gerium_public gerium_result_t
gerium_application_fullscreen(gerium_application_t application,
                              gerium_bool_t fullscreen,
                              gerium_uint32_t display_id,
                              const gerium_display_mode_t* mode);

gerium_public gerium_application_style_flags_t
gerium_application_get_style(gerium_application_t application);

gerium_public void
gerium_application_set_style(gerium_application_t application,
                             gerium_application_style_flags_t style);

gerium_public void
gerium_application_get_min_size(gerium_application_t application,
                                gerium_uint16_t* width,
                                gerium_uint16_t* height);

gerium_public void
gerium_application_set_min_size(gerium_application_t application,
                                gerium_uint16_t width,
                                gerium_uint16_t height);

gerium_public void
gerium_application_get_max_size(gerium_application_t application,
                                gerium_uint16_t* width,
                                gerium_uint16_t* height);

gerium_public void
gerium_application_set_max_size(gerium_application_t application,
                                gerium_uint16_t width,
                                gerium_uint16_t height);

gerium_public void
gerium_application_get_size(gerium_application_t application,
                            gerium_uint16_t* width,
                            gerium_uint16_t* height);

gerium_public void
gerium_application_set_size(gerium_application_t application,
                            gerium_uint16_t width,
                            gerium_uint16_t height);

gerium_public gerium_utf8_t
gerium_application_get_title(gerium_application_t application);

gerium_public void
gerium_application_set_title(gerium_application_t application,
                             gerium_utf8_t title);

gerium_public gerium_bool_t
gerium_application_get_background_wait(gerium_application_t application);

gerium_public void
gerium_application_set_background_wait(gerium_application_t application,
                                       gerium_bool_t enable);

gerium_public gerium_result_t
gerium_application_run(gerium_application_t application);

gerium_public void
gerium_application_exit(gerium_application_t application);

gerium_public gerium_result_t
gerium_renderer_create(gerium_application_t application,
                       gerium_uint32_t version,
                       gerium_bool_t debug,
                       gerium_renderer_t* renderer);

gerium_public gerium_renderer_t
gerium_renderer_reference(gerium_renderer_t renderer);

gerium_public void
gerium_renderer_destroy(gerium_renderer_t renderer);

gerium_public gerium_bool_t
gerium_renderer_get_profiler_enable(gerium_renderer_t renderer);

gerium_public void
gerium_renderer_set_profiler_enable(gerium_renderer_t renderer,
                                    gerium_bool_t enable);

gerium_public gerium_result_t
gerium_renderer_create_buffer(gerium_renderer_t renderer,
                              gerium_buffer_usage_flags_t buffer_usage,
                              gerium_bool_t dynamic,
                              gerium_utf8_t name,
                              gerium_cdata_t data,
                              gerium_uint32_t size,
                              gerium_buffer_h* handle);

gerium_public gerium_result_t
gerium_renderer_create_texture(gerium_renderer_t renderer,
                               const gerium_texture_creation_t* creation,
                               gerium_texture_h* handle);

gerium_public gerium_result_t
gerium_renderer_create_material(gerium_renderer_t renderer,
                                gerium_frame_graph_t frame_graph,
                                gerium_utf8_t name,
                                gerium_uint32_t pipeline_count,
                                const gerium_pipeline_t* pipelines,
                                gerium_material_h* handle);

gerium_public gerium_result_t
gerium_renderer_create_descriptor_set(gerium_renderer_t renderer,
                                      gerium_descriptor_set_h* handle);

gerium_public void
gerium_renderer_destroy_buffer(gerium_renderer_t renderer,
                               gerium_buffer_h handle);

gerium_public void
gerium_renderer_destroy_texture(gerium_renderer_t renderer,
                                gerium_texture_h handle);

gerium_public void
gerium_renderer_destroy_material(gerium_renderer_t renderer,
                                 gerium_material_h handle);

gerium_public void
gerium_renderer_destroy_descriptor_set(gerium_renderer_t renderer,
                                       gerium_descriptor_set_h handle);

gerium_public void
gerium_renderer_bind_buffer(gerium_renderer_t renderer,
                            gerium_descriptor_set_h handle,
                            gerium_uint16_t binding,
                            gerium_buffer_h buffer);

gerium_public void
gerium_renderer_bind_resource(gerium_renderer_t renderer,
                              gerium_descriptor_set_h handle,
                              gerium_uint16_t binding,
                              gerium_frame_graph_t frame_graph,
                              gerium_utf8_t name);

gerium_public gerium_data_t
gerium_renderer_map_buffer(gerium_renderer_t renderer,
                           gerium_buffer_h handle,
                           gerium_uint32_t offset,
                           gerium_uint32_t size);

gerium_public void
gerium_renderer_unmap_buffer(gerium_renderer_t renderer,
                             gerium_buffer_h handle);

gerium_public gerium_result_t
gerium_renderer_new_frame(gerium_renderer_t renderer);

gerium_public gerium_result_t
gerium_renderer_render(gerium_renderer_t renderer,
                       gerium_frame_graph_t frame_graph);

gerium_public gerium_result_t
gerium_renderer_present(gerium_renderer_t renderer);

gerium_public gerium_result_t
gerium_frame_graph_create(gerium_renderer_t renderer,
                          gerium_frame_graph_t* frame_graph);

gerium_public void
gerium_command_buffer_set_viewport(gerium_command_buffer_t command_buffer,
                                   gerium_uint16_t x,
                                   gerium_uint16_t y,
                                   gerium_uint16_t width,
                                   gerium_uint16_t height,
                                   gerium_float32_t min_depth,
                                   gerium_float32_t max_depth);

gerium_public void
gerium_command_buffer_set_scissor(gerium_command_buffer_t command_buffer,
                                  gerium_uint16_t x,
                                  gerium_uint16_t y,
                                  gerium_uint16_t width,
                                  gerium_uint16_t height);

gerium_public void
gerium_command_buffer_bind_material(gerium_command_buffer_t command_buffer,
                                    gerium_material_h handle);

gerium_public void
gerium_command_buffer_bind_vertex_buffer(gerium_command_buffer_t command_buffer,
                                         gerium_buffer_h handle,
                                         gerium_uint32_t binding,
                                         gerium_uint32_t offset);

gerium_public void
gerium_command_buffer_bind_descriptor_set(gerium_command_buffer_t command_buffer,
                                          gerium_descriptor_set_h handle,
                                          gerium_uint32_t set);

gerium_public void
gerium_command_buffer_draw(gerium_command_buffer_t command_buffer,
                           gerium_uint32_t first_vertex,
                           gerium_uint32_t vertex_count,
                           gerium_uint32_t first_instance,
                           gerium_uint32_t instance_count);

gerium_public void
gerium_command_buffer_draw_profiler(gerium_command_buffer_t command_buffer,
                                    gerium_bool_t* show);

gerium_public gerium_frame_graph_t
gerium_frame_graph_reference(gerium_frame_graph_t frame_graph);

gerium_public void
gerium_frame_graph_destroy(gerium_frame_graph_t frame_graph);

gerium_public gerium_result_t
gerium_frame_graph_add_pass(gerium_frame_graph_t frame_graph,
                            gerium_utf8_t name,
                            const gerium_render_pass_t* render_pass,
                            gerium_data_t* data);

gerium_public gerium_result_t
gerium_frame_graph_remove_pass(gerium_frame_graph_t frame_graph,
                               gerium_utf8_t name);

gerium_public gerium_result_t
gerium_frame_graph_add_node(gerium_frame_graph_t frame_graph,
                            gerium_utf8_t name,
                            gerium_uint32_t input_count,
                            const gerium_resource_input_t* inputs,
                            gerium_uint32_t output_count,
                            const gerium_resource_output_t* outputs);

gerium_public void
gerium_frame_graph_clear(gerium_frame_graph_t frame_graph);

gerium_public gerium_result_t
gerium_frame_graph_compile(gerium_frame_graph_t frame_graph);

gerium_public gerium_result_t
gerium_profiler_create(gerium_renderer_t renderer,
                       gerium_profiler_t* profiler);

gerium_public gerium_profiler_t
gerium_profiler_reference(gerium_profiler_t profiler);

gerium_public void
gerium_profiler_destroy(gerium_profiler_t profiler);

gerium_public void
gerium_profiler_get_gpu_timestamps(gerium_profiler_t profiler,
                                   gerium_uint32_t* gpu_timestamps_count,
                                   gerium_gpu_timestamp_t* gpu_timestamps);

gerium_public gerium_uint32_t
gerium_profiler_get_gpu_total_memory_used(gerium_profiler_t profiler);

GERIUM_END

#endif
