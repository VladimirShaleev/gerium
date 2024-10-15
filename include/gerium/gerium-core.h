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
GERIUM_TYPE(gerium_file)
GERIUM_TYPE(gerium_mutex)
GERIUM_TYPE(gerium_signal)
GERIUM_TYPE(gerium_application)
GERIUM_TYPE(gerium_renderer)
GERIUM_TYPE(gerium_command_buffer)
GERIUM_TYPE(gerium_frame_graph)
GERIUM_TYPE(gerium_profiler)

GERIUM_HANDLE(gerium_buffer)
GERIUM_HANDLE(gerium_texture)
GERIUM_HANDLE(gerium_technique)
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

typedef enum
{
    GERIUM_FILE_SEEK_BEGIN    = 0,
    GERIUM_FILE_SEEK_CURRENT  = 1,
    GERIUM_FILE_SEEK_END      = 2,
    GERIUM_FILE_SEEK_MAX_ENUM = 0x7FFFFFFF
} gerium_file_seek_t;

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
    GERIUM_EVENT_TYPE_KEYBOARD = 0,
    GERIUM_EVENT_TYPE_MOUSE    = 1,
    GERIUM_EVENT_TYPE_MAX_ENUM = 0x7FFFFFFF
} gerium_event_type_t;

typedef enum
{
    GERIUM_KEY_STATE_PRESSED  = 0,
    GERIUM_KEY_STATE_RELEASED = 1,
    GERIUM_KEY_STATE_MAX_ENUM = 0x7FFFFFFF
} gerium_key_state_t;

typedef enum
{
    GERIUM_KEY_MOD_NONE        = 0,
    GERIUM_KEY_MOD_LSHIFT      = 1,
    GERIUM_KEY_MOD_RSHIFT      = 2,
    GERIUM_KEY_MOD_LCTRL       = 4,
    GERIUM_KEY_MOD_RCTRL       = 8,
    GERIUM_KEY_MOD_LALT        = 16,
    GERIUM_KEY_MOD_RALT        = 32,
    GERIUM_KEY_MOD_LMETA       = 64,
    GERIUM_KEY_MOD_RMETA       = 128,
    GERIUM_KEY_MOD_NUM_LOCK    = 256,
    GERIUM_KEY_MOD_CAPS_LOCK   = 512,
    GERIUM_KEY_MOD_SCROLL_LOCK = 1024,
    GERIUM_KEY_MOD_SHIFT       = GERIUM_KEY_MOD_LSHIFT | GERIUM_KEY_MOD_RSHIFT,
    GERIUM_KEY_MOD_CTRL        = GERIUM_KEY_MOD_LCTRL | GERIUM_KEY_MOD_RCTRL, 
    GERIUM_KEY_MOD_ALT         = GERIUM_KEY_MOD_LALT | GERIUM_KEY_MOD_RALT,
    GERIUM_KEY_MOD_META        = GERIUM_KEY_MOD_LMETA | GERIUM_KEY_MOD_RMETA,
    GERIUM_KEY_MOD_MAX_ENUM    = 0x7FFFFFFF 
} gerium_key_mod_flags_t;
GERIUM_FLAGS(gerium_key_mod_flags_t)

typedef enum
{
    GERIUM_SCANCODE_UNKNOWN              = 0,
    GERIUM_SCANCODE_0                    = 1,
    GERIUM_SCANCODE_1                    = 2,
    GERIUM_SCANCODE_2                    = 3,
    GERIUM_SCANCODE_3                    = 4,
    GERIUM_SCANCODE_4                    = 5,
    GERIUM_SCANCODE_5                    = 6,
    GERIUM_SCANCODE_6                    = 7,
    GERIUM_SCANCODE_7                    = 8,
    GERIUM_SCANCODE_8                    = 9,
    GERIUM_SCANCODE_9                    = 10,
    GERIUM_SCANCODE_A                    = 11,
    GERIUM_SCANCODE_B                    = 12,
    GERIUM_SCANCODE_C                    = 13,
    GERIUM_SCANCODE_D                    = 14,
    GERIUM_SCANCODE_E                    = 15,
    GERIUM_SCANCODE_F                    = 16,
    GERIUM_SCANCODE_G                    = 17,
    GERIUM_SCANCODE_H                    = 18,
    GERIUM_SCANCODE_I                    = 19,
    GERIUM_SCANCODE_J                    = 20,
    GERIUM_SCANCODE_K                    = 21,
    GERIUM_SCANCODE_L                    = 22,
    GERIUM_SCANCODE_M                    = 23,
    GERIUM_SCANCODE_N                    = 24,
    GERIUM_SCANCODE_O                    = 25,
    GERIUM_SCANCODE_P                    = 26,
    GERIUM_SCANCODE_Q                    = 27,
    GERIUM_SCANCODE_R                    = 28,
    GERIUM_SCANCODE_S                    = 29,
    GERIUM_SCANCODE_T                    = 30,
    GERIUM_SCANCODE_U                    = 31,
    GERIUM_SCANCODE_V                    = 32,
    GERIUM_SCANCODE_W                    = 33,
    GERIUM_SCANCODE_X                    = 34,
    GERIUM_SCANCODE_Y                    = 35,
    GERIUM_SCANCODE_Z                    = 36,
    GERIUM_SCANCODE_F1                   = 37,
    GERIUM_SCANCODE_F2                   = 38,
    GERIUM_SCANCODE_F3                   = 39,
    GERIUM_SCANCODE_F4                   = 40,
    GERIUM_SCANCODE_F5                   = 41,
    GERIUM_SCANCODE_F6                   = 42,
    GERIUM_SCANCODE_F7                   = 43,
    GERIUM_SCANCODE_F8                   = 44,
    GERIUM_SCANCODE_F9                   = 45,
    GERIUM_SCANCODE_F10                  = 46,
    GERIUM_SCANCODE_F11                  = 47,
    GERIUM_SCANCODE_F12                  = 48,
    GERIUM_SCANCODE_F13                  = 49,
    GERIUM_SCANCODE_F14                  = 50,
    GERIUM_SCANCODE_F15                  = 51,
    GERIUM_SCANCODE_F16                  = 52,
    GERIUM_SCANCODE_F17                  = 53,
    GERIUM_SCANCODE_F18                  = 54,
    GERIUM_SCANCODE_F19                  = 55,
    GERIUM_SCANCODE_F20                  = 56,
    GERIUM_SCANCODE_F21                  = 57,
    GERIUM_SCANCODE_F22                  = 58,
    GERIUM_SCANCODE_F23                  = 59,
    GERIUM_SCANCODE_F24                  = 60,
    GERIUM_SCANCODE_NUMPAD_0             = 61,
    GERIUM_SCANCODE_NUMPAD_1             = 62,
    GERIUM_SCANCODE_NUMPAD_2             = 63,
    GERIUM_SCANCODE_NUMPAD_3             = 64,
    GERIUM_SCANCODE_NUMPAD_4             = 65,
    GERIUM_SCANCODE_NUMPAD_5             = 66,
    GERIUM_SCANCODE_NUMPAD_6             = 67,
    GERIUM_SCANCODE_NUMPAD_7             = 68,
    GERIUM_SCANCODE_NUMPAD_8             = 69,
    GERIUM_SCANCODE_NUMPAD_9             = 70,
    GERIUM_SCANCODE_NUMPAD_COMMA         = 71,
    GERIUM_SCANCODE_NUMPAD_ENTER         = 72,
    GERIUM_SCANCODE_NUMPAD_EQUAL         = 73,
    GERIUM_SCANCODE_NUMPAD_SUBTRACT      = 74,
    GERIUM_SCANCODE_NUMPAD_DECIMAL       = 75,
    GERIUM_SCANCODE_NUMPAD_ADD           = 76,
    GERIUM_SCANCODE_NUMPAD_DIVIDE        = 77,
    GERIUM_SCANCODE_NUMPAD_MULTIPLY      = 78,
    GERIUM_SCANCODE_NUM_LOCK             = 79,
    GERIUM_SCANCODE_MEDIA_PLAY_PAUSE     = 80,
    GERIUM_SCANCODE_MEDIA_TRACK_PREVIOUS = 81,
    GERIUM_SCANCODE_MEDIA_TRACK_NEXT     = 82,
    GERIUM_SCANCODE_MEDIA_STOP           = 83,
    GERIUM_SCANCODE_LAUNCH_MEDIA_PLAYER  = 84,
    GERIUM_SCANCODE_AUDIO_VOLUME_MUTE    = 85,
    GERIUM_SCANCODE_AUDIO_VOLUME_DOWN    = 86,
    GERIUM_SCANCODE_AUDIO_VOLUME_UP      = 87,
    GERIUM_SCANCODE_ESCAPE               = 88,
    GERIUM_SCANCODE_TAB                  = 89,
    GERIUM_SCANCODE_CAPS_LOCK            = 90,
    GERIUM_SCANCODE_ENTER                = 91,
    GERIUM_SCANCODE_BACKSLASH            = 92,
    GERIUM_SCANCODE_BACKSPACE            = 93,
    GERIUM_SCANCODE_INTL_BACKSLASH       = 94,
    GERIUM_SCANCODE_INTL_RO              = 95,
    GERIUM_SCANCODE_INTL_YEN             = 96,
    GERIUM_SCANCODE_MINUS                = 97,
    GERIUM_SCANCODE_COLON                = 98,
    GERIUM_SCANCODE_COMMA                = 99,
    GERIUM_SCANCODE_CONVERT              = 100,
    GERIUM_SCANCODE_NONCONVERT           = 101,
    GERIUM_SCANCODE_EQUAL                = 102,
    GERIUM_SCANCODE_PERIOD               = 103,
    GERIUM_SCANCODE_POWER                = 104,
    GERIUM_SCANCODE_SEMICOLON            = 105,
    GERIUM_SCANCODE_SLASH                = 106,
    GERIUM_SCANCODE_SLEEP                = 107,
    GERIUM_SCANCODE_WAKE                 = 108,
    GERIUM_SCANCODE_SPACE                = 109,
    GERIUM_SCANCODE_QUOTE                = 110,
    GERIUM_SCANCODE_BACKQUOTE            = 111,
    GERIUM_SCANCODE_ALT_LEFT             = 112,
    GERIUM_SCANCODE_ALT_RIGHT            = 113,
    GERIUM_SCANCODE_BRACKET_LEFT         = 114,
    GERIUM_SCANCODE_BRACKET_RIGHT        = 115,
    GERIUM_SCANCODE_CONTROL_LEFT         = 116,
    GERIUM_SCANCODE_CONTROL_RIGHT        = 117,
    GERIUM_SCANCODE_SHIFT_LEFT           = 118,
    GERIUM_SCANCODE_SHIFT_RIGHT          = 119,
    GERIUM_SCANCODE_META_LEFT            = 120,
    GERIUM_SCANCODE_META_RIGHT           = 121,
    GERIUM_SCANCODE_ARROW_UP             = 122,
    GERIUM_SCANCODE_ARROW_LEFT           = 123,
    GERIUM_SCANCODE_ARROW_RIGHT          = 124,
    GERIUM_SCANCODE_ARROW_DOWN           = 125,
    GERIUM_SCANCODE_SCROLL_LOCK          = 126,
    GERIUM_SCANCODE_PAUSE                = 127,
    GERIUM_SCANCODE_CTRL_PAUSE           = 128,
    GERIUM_SCANCODE_INSERT               = 129,
    GERIUM_SCANCODE_DELETE               = 130,
    GERIUM_SCANCODE_HOME                 = 131,
    GERIUM_SCANCODE_END                  = 132,
    GERIUM_SCANCODE_PAGE_UP              = 133,
    GERIUM_SCANCODE_PAGE_DOWN            = 134,
    GERIUM_SCANCODE_LAUNCH_MAIL          = 135,
    GERIUM_SCANCODE_MYCOMPUTER           = 136,
    GERIUM_SCANCODE_CONTEXT_MENU         = 137,
    GERIUM_SCANCODE_PRINT_SCREEN         = 138,
    GERIUM_SCANCODE_ALT_PRINT_SCREEN     = 139,
    GERIUM_SCANCODE_LAUNCH_APPLICATION_1 = 140,
    GERIUM_SCANCODE_LAUNCH_APPLICATION_2 = 141,
    GERIUM_SCANCODE_KANA_MODE            = 142,
    GERIUM_SCANCODE_BROWSER_BACK         = 143,
    GERIUM_SCANCODE_BROWSER_FAVORITES    = 144,
    GERIUM_SCANCODE_BROWSER_FORWARD      = 145,
    GERIUM_SCANCODE_BROWSER_HOME         = 146,
    GERIUM_SCANCODE_BROWSER_REFRESH      = 147,
    GERIUM_SCANCODE_BROWSER_SEARCH       = 148,
    GERIUM_SCANCODE_BROWSER_STOP         = 149,
    GERIUM_SCANCODE_MAX_ENUM             = 0x7FFFFFFF
} gerium_scancode_t;

typedef enum
{
    GERIUM_KEY_CODE_UNKNOWN              = 0,
    GERIUM_KEY_CODE_0                    = 1,
    GERIUM_KEY_CODE_1                    = 2,
    GERIUM_KEY_CODE_2                    = 3,
    GERIUM_KEY_CODE_3                    = 4,
    GERIUM_KEY_CODE_4                    = 5,
    GERIUM_KEY_CODE_5                    = 6,
    GERIUM_KEY_CODE_6                    = 7,
    GERIUM_KEY_CODE_7                    = 8,
    GERIUM_KEY_CODE_8                    = 9,
    GERIUM_KEY_CODE_9                    = 10,
    GERIUM_KEY_CODE_A                    = 11,
    GERIUM_KEY_CODE_B                    = 12,
    GERIUM_KEY_CODE_C                    = 13,
    GERIUM_KEY_CODE_D                    = 14,
    GERIUM_KEY_CODE_E                    = 15,
    GERIUM_KEY_CODE_F                    = 16,
    GERIUM_KEY_CODE_G                    = 17,
    GERIUM_KEY_CODE_H                    = 18,
    GERIUM_KEY_CODE_I                    = 19,
    GERIUM_KEY_CODE_J                    = 20,
    GERIUM_KEY_CODE_K                    = 21,
    GERIUM_KEY_CODE_L                    = 22,
    GERIUM_KEY_CODE_M                    = 23,
    GERIUM_KEY_CODE_N                    = 24,
    GERIUM_KEY_CODE_O                    = 25,
    GERIUM_KEY_CODE_P                    = 26,
    GERIUM_KEY_CODE_Q                    = 27,
    GERIUM_KEY_CODE_R                    = 28,
    GERIUM_KEY_CODE_S                    = 29,
    GERIUM_KEY_CODE_T                    = 30,
    GERIUM_KEY_CODE_U                    = 31,
    GERIUM_KEY_CODE_V                    = 32,
    GERIUM_KEY_CODE_W                    = 33,
    GERIUM_KEY_CODE_X                    = 34,
    GERIUM_KEY_CODE_Y                    = 35,
    GERIUM_KEY_CODE_Z                    = 36,
    GERIUM_KEY_CODE_F1                   = 37,
    GERIUM_KEY_CODE_F2                   = 38,
    GERIUM_KEY_CODE_F3                   = 39,
    GERIUM_KEY_CODE_F4                   = 40,
    GERIUM_KEY_CODE_F5                   = 41,
    GERIUM_KEY_CODE_F6                   = 42,
    GERIUM_KEY_CODE_F7                   = 43,
    GERIUM_KEY_CODE_F8                   = 44,
    GERIUM_KEY_CODE_F9                   = 45,
    GERIUM_KEY_CODE_F10                  = 46,
    GERIUM_KEY_CODE_F11                  = 47,
    GERIUM_KEY_CODE_F12                  = 48,
    GERIUM_KEY_CODE_F13                  = 49,
    GERIUM_KEY_CODE_F14                  = 50,
    GERIUM_KEY_CODE_F15                  = 51,
    GERIUM_KEY_CODE_F16                  = 52,
    GERIUM_KEY_CODE_F17                  = 53,
    GERIUM_KEY_CODE_F18                  = 54,
    GERIUM_KEY_CODE_F19                  = 55,
    GERIUM_KEY_CODE_F20                  = 56,
    GERIUM_KEY_CODE_F21                  = 57,
    GERIUM_KEY_CODE_F22                  = 58,
    GERIUM_KEY_CODE_F23                  = 59,
    GERIUM_KEY_CODE_F24                  = 60,
    GERIUM_KEY_CODE_EXCLAIM              = 61,
    GERIUM_KEY_CODE_AT                   = 62,
    GERIUM_KEY_CODE_HASH                 = 63,
    GERIUM_KEY_CODE_DOLLAR               = 64,
    GERIUM_KEY_CODE_PERCENT              = 65,
    GERIUM_KEY_CODE_CARET                = 66,
    GERIUM_KEY_CODE_AMPERSAND            = 67,
    GERIUM_KEY_CODE_ASTERISK             = 68,
    GERIUM_KEY_CODE_PAREN_LEFT           = 69,
    GERIUM_KEY_CODE_PAREN_RIGHT          = 70,
    GERIUM_KEY_CODE_UNDERSCORE           = 71,
    GERIUM_KEY_CODE_NUMPAD_0             = 72,
    GERIUM_KEY_CODE_NUMPAD_1             = 73,
    GERIUM_KEY_CODE_NUMPAD_2             = 74,
    GERIUM_KEY_CODE_NUMPAD_3             = 75,
    GERIUM_KEY_CODE_NUMPAD_4             = 76,
    GERIUM_KEY_CODE_NUMPAD_5             = 77,
    GERIUM_KEY_CODE_NUMPAD_6             = 78,
    GERIUM_KEY_CODE_NUMPAD_7             = 79,
    GERIUM_KEY_CODE_NUMPAD_8             = 80,
    GERIUM_KEY_CODE_NUMPAD_9             = 81,
    GERIUM_KEY_CODE_DECIMAL              = 82,
    GERIUM_KEY_CODE_SUBTRACT             = 83,
    GERIUM_KEY_CODE_ADD                  = 84,
    GERIUM_KEY_CODE_DIVIDE               = 85,
    GERIUM_KEY_CODE_MULTIPLY             = 86,
    GERIUM_KEY_CODE_EQUAL                = 87,
    GERIUM_KEY_CODE_LESS                 = 88,
    GERIUM_KEY_CODE_GREATER              = 89,
    GERIUM_KEY_CODE_NUM_LOCK             = 90,
    GERIUM_KEY_CODE_MEDIA_PLAY_PAUSE     = 91,
    GERIUM_KEY_CODE_MEDIA_TRACK_PREVIOUS = 92,
    GERIUM_KEY_CODE_MEDIA_TRACK_NEXT     = 93,
    GERIUM_KEY_CODE_MEDIA_STOP           = 94,
    GERIUM_KEY_CODE_LAUNCH_MEDIA_PLAYER  = 95,
    GERIUM_KEY_CODE_AUDIO_VOLUME_MUTE    = 96,
    GERIUM_KEY_CODE_AUDIO_VOLUME_DOWN    = 97,
    GERIUM_KEY_CODE_AUDIO_VOLUME_UP      = 98,
    GERIUM_KEY_CODE_ESCAPE               = 99,
    GERIUM_KEY_CODE_TAB                  = 100,
    GERIUM_KEY_CODE_CAPS_LOCK            = 101,
    GERIUM_KEY_CODE_ENTER                = 102,
    GERIUM_KEY_CODE_BACKSLASH            = 103,
    GERIUM_KEY_CODE_PIPE                 = 104,
    GERIUM_KEY_CODE_QUESTION             = 105,
    GERIUM_KEY_CODE_BACKSPACE            = 106,
    GERIUM_KEY_CODE_COLON                = 107,
    GERIUM_KEY_CODE_COMMA                = 108,
    GERIUM_KEY_CODE_CONVERT              = 109,
    GERIUM_KEY_CODE_NONCONVERT           = 110,
    GERIUM_KEY_CODE_PERIOD               = 111,
    GERIUM_KEY_CODE_POWER                = 112,
    GERIUM_KEY_CODE_SEMICOLON            = 113,
    GERIUM_KEY_CODE_SLASH                = 114,
    GERIUM_KEY_CODE_SLEEP                = 115,
    GERIUM_KEY_CODE_WAKE                 = 116,
    GERIUM_KEY_CODE_SPACE                = 117,
    GERIUM_KEY_CODE_DOUBLE_QUOTE         = 118,
    GERIUM_KEY_CODE_QUOTE                = 119,
    GERIUM_KEY_CODE_BACKQUOTE            = 120,
    GERIUM_KEY_CODE_TILDE                = 121,
    GERIUM_KEY_CODE_ALT_LEFT             = 122,
    GERIUM_KEY_CODE_ALT_RIGHT            = 123,
    GERIUM_KEY_CODE_BRACKET_LEFT         = 124,
    GERIUM_KEY_CODE_BRACKET_RIGHT        = 125,
    GERIUM_KEY_CODE_BRACE_LEFT           = 126,
    GERIUM_KEY_CODE_BRACE_RIGHT          = 127,
    GERIUM_KEY_CODE_CONTROL_LEFT         = 128,
    GERIUM_KEY_CODE_CONTROL_RIGHT        = 129,
    GERIUM_KEY_CODE_SHIFT_LEFT           = 130,
    GERIUM_KEY_CODE_SHIFT_RIGHT          = 131,
    GERIUM_KEY_CODE_META_LEFT            = 132,
    GERIUM_KEY_CODE_META_RIGHT           = 133,
    GERIUM_KEY_CODE_ARROW_UP             = 134,
    GERIUM_KEY_CODE_ARROW_LEFT           = 135,
    GERIUM_KEY_CODE_ARROW_RIGHT          = 136,
    GERIUM_KEY_CODE_ARROW_DOWN           = 137,
    GERIUM_KEY_CODE_SCROLL_LOCK          = 138,
    GERIUM_KEY_CODE_PAUSE                = 139,
    GERIUM_KEY_CODE_INSERT               = 140,
    GERIUM_KEY_CODE_DELETE               = 141,
    GERIUM_KEY_CODE_HOME                 = 142,
    GERIUM_KEY_CODE_END                  = 143,
    GERIUM_KEY_CODE_PAGE_UP              = 144,
    GERIUM_KEY_CODE_PAGE_DOWN            = 145,
    GERIUM_KEY_CODE_LAUNCH_MAIL          = 146,
    GERIUM_KEY_CODE_CONTEXT_MENU         = 147,
    GERIUM_KEY_CODE_PRINT_SCREEN         = 148,
    GERIUM_KEY_CODE_LAUNCH_APPLICATION_1 = 149,
    GERIUM_KEY_CODE_LAUNCH_APPLICATION_2 = 150,
    GERIUM_KEY_CODE_KANA_MODE            = 151,
    GERIUM_KEY_CODE_BROWSER_BACK         = 152,
    GERIUM_KEY_CODE_BROWSER_FAVORITES    = 153,
    GERIUM_KEY_CODE_BROWSER_FORWARD      = 154,
    GERIUM_KEY_CODE_BROWSER_HOME         = 155,
    GERIUM_KEY_CODE_BROWSER_REFRESH      = 156,
    GERIUM_KEY_CODE_BROWSER_SEARCH       = 157,
    GERIUM_KEY_CODE_BROWSER_STOP         = 158,
    GERIUM_KEY_CODE_MAX_ENUM             = 0x7FFFFFFF
} gerium_key_code_t;

typedef enum
{
    GERIUM_MOUSE_BUTTON_NONE        = 0,
    GERIUM_MOUSE_BUTTON_LEFT_DOWN   = 1,
    GERIUM_MOUSE_BUTTON_LEFT_UP     = 2,
    GERIUM_MOUSE_BUTTON_RIGHT_DOWN  = 4,
    GERIUM_MOUSE_BUTTON_RIGHT_UP    = 8,
    GERIUM_MOUSE_BUTTON_MIDDLE_DOWN = 16,
    GERIUM_MOUSE_BUTTON_MIDDLE_UP   = 32,
    GERIUM_MOUSE_BUTTON_4_DOWN      = 64,
    GERIUM_MOUSE_BUTTON_4_UP        = 128,
    GERIUM_MOUSE_BUTTON_5_DOWN      = 256,
    GERIUM_MOUSE_BUTTON_5_UP        = 512,
    GERIUM_MOUSE_BUTTON_MAX_ENUM = 0x7FFFFFFF
} gerium_mouse_button_flags_t;
GERIUM_FLAGS(gerium_mouse_button_flags_t)

typedef enum
{
    GERIUM_FEATURE_NONE_BIT           = 0,
    GERIUM_FEATURE_BINDLESS_BIT       = 1,
    GERIUM_FEATURE_MESH_SHADER_BIT    = 2,
    GERIUM_FEATURE_8_BIT_STORAGE_BIT  = 4,
    GERIUM_FEATURE_16_BIT_STORAGE_BIT = 8,
    GERIUM_FEATURE_MAX_ENUM           = 0x7FFFFFFF
} gerium_feature_flags_t;
GERIUM_FLAGS(gerium_feature_flags_t)

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
    GERIUM_PRIMITIVE_TOPOLOGY_POINT_LIST     = 0,
    GERIUM_PRIMITIVE_TOPOLOGY_LINE_LIST      = 1,
    GERIUM_PRIMITIVE_TOPOLOGY_LINE_STRIP     = 2,
    GERIUM_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST  = 3,
    GERIUM_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
    GERIUM_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN   = 5,
    GERIUM_PRIMITIVE_TOPOLOGY_MAX_ENUM       = 0x7FFFFFFF
} gerium_primitive_topology_t;

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
    GERIUM_BUFFER_USAGE_NONE_BIT     = 0,
    GERIUM_BUFFER_USAGE_VERTEX_BIT   = 1,
    GERIUM_BUFFER_USAGE_INDEX_BIT    = 2,
    GERIUM_BUFFER_USAGE_UNIFORM_BIT  = 4,
    GERIUM_BUFFER_USAGE_STORAGE_BIT  = 8,
    GERIUM_BUFFER_USAGE_INDIRECT_BIT = 16,
    GERIUM_BUFFER_USAGE_MAX_ENUM     = 0x7FFFFFFF
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
    GERIUM_FILTER_NEAREST  = 0,
    GERIUM_FILTER_LINEAR   = 1,
    GERIUM_FILTER_MAX_ENUM = 0x7FFFFFFF
} gerium_filter_t;

typedef enum
{
    GERIUM_ADDRESS_MODE_REPEAT               = 0,
    GERIUM_ADDRESS_MODE_MIRRORED_REPEAT      = 1,
    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE        = 2,
    GERIUM_ADDRESS_MODE_CLAMP_TO_BORDER      = 3,
    GERIUM_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE = 4,
    GERIUM_ADDRESS_MODE_MAX_ENUM             = 0x7FFFFFFF
} gerium_address_mode_t;

typedef enum
{
    GERIUM_RESOURCE_TYPE_BUFFER     = 0,
    GERIUM_RESOURCE_TYPE_TEXTURE    = 1,
    GERIUM_RESOURCE_TYPE_ATTACHMENT = 2,
    GERIUM_RESOURCE_TYPE_REFERENCE  = 3,
    GERIUM_RESOURCE_TYPE_MAX_ENUM   = 0x7FFFFFFF
} gerium_resource_type_t;

typedef enum
{
    GERIUM_VERTEX_RATE_PER_VERTEX   = 0,
    GERIUM_VERTEX_RATE_PER_INSTANCE = 1,
    GERIUM_VERTEX_RATE_MAX_ENUM     = 0x7FFFFFFF
} gerium_vertex_rate_t;

typedef enum
{
    GERIUM_INDEX_TYPE_UINT16   = 0,
    GERIUM_INDEX_TYPE_UINT32   = 1,
    GERIUM_INDEX_TYPE_MAX_ENUM = 0x7FFFFFFF
} gerium_index_type_t;

typedef enum {
    GERIUM_SHADER_TYPE_VERTEX   = 0,
    GERIUM_SHADER_TYPE_FRAGMENT = 1,
    GERIUM_SHADER_TYPE_COMPUTE  = 2,
    GERIUM_SHADER_TYPE_TASK     = 3,
    GERIUM_SHADER_TYPE_MESH     = 4,
    GERIUM_SHADER_TYPE_MAX_ENUM = 0x7FFFFFFF
} gerium_shader_type_t;

typedef enum
{
    GERIUM_SHADER_LANGUAGE_UNKNOWN = 0,
    GERIUM_SHADER_LANGUAGE_SPIRV   = 1,
    GERIUM_SHADER_LANGUAGE_GLSL    = 2,
    GERIUM_SHADER_LANGUAGE_HLSL    = 3,
    GERIUM_SHADER_LANGUAGE_MAX_ENUM = 0x7FFFFFFF
} gerium_shader_languge_t;

typedef gerium_bool_t
(*gerium_application_frame_func_t)(gerium_application_t application,
                                   gerium_data_t data,
                                   gerium_uint64_t elapsed_ms);

typedef gerium_bool_t
(*gerium_application_state_func_t)(gerium_application_t application,
                                   gerium_data_t data,
                                   gerium_application_state_t state);

typedef void
(*gerium_application_executor_func_t)(gerium_application_t application,
                                      gerium_data_t data);

typedef gerium_uint32_t
(*gerium_frame_graph_prepare_func_t)(gerium_frame_graph_t frame_graph,
                                     gerium_renderer_t renderer,
                                     gerium_uint32_t max_workers,
                                     gerium_data_t data);

typedef gerium_bool_t
(*gerium_frame_graph_resize_func_t)(gerium_frame_graph_t frame_graph,
                                    gerium_renderer_t renderer,
                                    gerium_data_t data);

typedef gerium_bool_t
(*gerium_frame_graph_render_func_t)(gerium_frame_graph_t frame_graph,
                                    gerium_renderer_t renderer,
                                    gerium_command_buffer_t command_buffer,
                                    gerium_uint32_t worker,
                                    gerium_uint32_t total_workers,
                                    gerium_data_t data);

typedef void
(*gerium_texture_loaded_func_t)(gerium_renderer_t renderer,
                                gerium_texture_h texture,
                                gerium_data_t data);

typedef struct
{
    gerium_scancode_t      scancode;
    gerium_key_code_t      code;
    gerium_key_state_t     state;
    gerium_key_mod_flags_t modifiers;
    gerium_char_t          symbol[5];
} gerium_keyboard_event_t;

typedef struct
{
    gerium_uint32_t             id;
    gerium_mouse_button_flags_t buttons;
    gerium_sint16_t             absolute_x;
    gerium_sint16_t             absolute_y;
    gerium_sint16_t             delta_x;
    gerium_sint16_t             delta_y;
    gerium_sint16_t             raw_delta_x;
    gerium_sint16_t             raw_delta_y;
    gerium_float32_t            wheel_vertical;
    gerium_float32_t            wheel_horizontal;
} gerium_mouse_event_t;

typedef struct
{
    gerium_event_type_t     type;
    gerium_uint64_t         timestamp;
    gerium_keyboard_event_t keyboard;
    gerium_mouse_event_t    mouse;
} gerium_event_t;

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
    gerium_utf8_t         name;
} gerium_texture_info_t;

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
    gerium_polygon_mode_t       polygon_mode;
    gerium_primitive_topology_t primitive_topology;
    gerium_cull_mode_t          cull_mode;
    gerium_front_face_t         front_face;
    gerium_bool_t               depth_clamp_enable;
    gerium_bool_t               depth_bias_enable;
    gerium_float32_t            depth_bias_constant_factor;
    gerium_float32_t            depth_bias_clamp;
    gerium_float32_t            depth_bias_slope_factor;
    gerium_float32_t            line_width;
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
    gerium_uint16_t      binding;
    gerium_uint16_t      stride;
    gerium_vertex_rate_t input_rate;
} gerium_vertex_binding_t;

typedef struct
{
    gerium_char_t name[128];
    gerium_char_t value[128];
} gerium_macro_definition_t;

typedef struct
{
    gerium_shader_type_t             type;
    gerium_shader_languge_t          lang;
    gerium_utf8_t                    name;
    gerium_cdata_t                   data;
    gerium_uint32_t                  size;
    gerium_uint32_t                  macro_count;
    const gerium_macro_definition_t* macros;
} gerium_shader_t;

typedef struct
{
    gerium_resource_type_t type;
    gerium_utf8_t          name;
    gerium_bool_t          previous_frame;
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
    gerium_uint32_t                               size;
    gerium_buffer_usage_flags_t                   usage;
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

gerium_public gerium_utf8_t
gerium_file_get_cache_dir(void);

gerium_public gerium_utf8_t
gerium_file_get_app_dir(void);

gerium_public gerium_bool_t
gerium_file_exists_file(gerium_utf8_t path);

gerium_public gerium_bool_t
gerium_file_exists_dir(gerium_utf8_t path);

gerium_public void
gerium_file_delete_file(gerium_utf8_t path);

gerium_public gerium_result_t
gerium_file_open(gerium_utf8_t path,
                 gerium_bool_t read_only,
                 gerium_file_t* file);

gerium_public gerium_result_t
gerium_file_create(gerium_utf8_t path,
                   gerium_uint64_t size,
                   gerium_file_t* file);

gerium_public gerium_result_t
gerium_file_create_temp(gerium_uint64_t size,
                        gerium_file_t* file);

gerium_public gerium_file_t
gerium_file_reference(gerium_file_t file);

gerium_public void
gerium_file_destroy(gerium_file_t file);

gerium_public gerium_uint64_t
gerium_file_get_size(gerium_file_t file);

gerium_public void
gerium_file_seek(gerium_file_t file,
                 gerium_uint64_t offset,
                 gerium_file_seek_t seek);

gerium_public gerium_result_t
gerium_file_write(gerium_file_t file,
                  gerium_cdata_t data,
                  gerium_uint32_t size);

gerium_public gerium_uint32_t
gerium_file_read(gerium_file_t file,
                 gerium_data_t data,
                 gerium_uint32_t size);

gerium_public gerium_data_t
gerium_file_map(gerium_file_t file);

gerium_public gerium_result_t
gerium_mutex_create(gerium_mutex_t* mutex);

gerium_public gerium_mutex_t
gerium_mutex_reference(gerium_mutex_t mutex);

gerium_public void
gerium_mutex_destroy(gerium_mutex_t mutex);

gerium_public void
gerium_mutex_lock(gerium_mutex_t mutex);

gerium_public void
gerium_mutex_unlock(gerium_mutex_t mutex);

gerium_public gerium_result_t
gerium_signal_create(gerium_signal_t* signal);

gerium_public gerium_signal_t
gerium_signal_reference(gerium_signal_t signal);

gerium_public void
gerium_signal_destroy(gerium_signal_t signal);

gerium_public gerium_bool_t
gerium_signal_is_set(gerium_signal_t signal);

gerium_public void
gerium_signal_set(gerium_signal_t signal);

gerium_public void
gerium_signal_wait(gerium_signal_t signal);

gerium_public void
gerium_signal_clear(gerium_signal_t signal);

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

gerium_public gerium_bool_t
gerium_application_is_show_cursor(gerium_application_t application);

gerium_public void
gerium_application_show_cursor(gerium_application_t application,
                               gerium_bool_t show);

gerium_public gerium_result_t
gerium_application_run(gerium_application_t application);

gerium_public void
gerium_application_exit(gerium_application_t application);

gerium_public gerium_bool_t
gerium_application_poll_events(gerium_application_t application,
                               gerium_event_t* event);

gerium_public gerium_bool_t
gerium_application_is_press_scancode(gerium_application_t application,
                                     gerium_scancode_t scancode);

gerium_public void
gerium_application_execute(gerium_application_t application,
                           gerium_application_executor_func_t callback,
                           gerium_data_t data);

gerium_public gerium_result_t
gerium_renderer_create(gerium_application_t application,
                       gerium_feature_flags_t features,
                       gerium_uint32_t version,
                       gerium_bool_t debug,
                       gerium_renderer_t* renderer);

gerium_public gerium_renderer_t
gerium_renderer_reference(gerium_renderer_t renderer);

gerium_public void
gerium_renderer_destroy(gerium_renderer_t renderer);

gerium_public gerium_feature_flags_t
gerium_renderer_get_enabled_features(gerium_renderer_t renderer);

gerium_public gerium_bool_t
gerium_renderer_get_profiler_enable(gerium_renderer_t renderer);

gerium_public void
gerium_renderer_set_profiler_enable(gerium_renderer_t renderer,
                                    gerium_bool_t enable);

gerium_public gerium_bool_t
gerium_renderer_is_supported_format(gerium_renderer_t renderer,
                                    gerium_format_t format);

gerium_public void
gerium_renderer_get_texture_info(gerium_renderer_t renderer,
                                 gerium_texture_h handle,
                                 gerium_texture_info_t* info);

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
                               const gerium_texture_info_t* info,
                               gerium_cdata_t data,
                               gerium_texture_h* handle);

gerium_public gerium_result_t
gerium_renderer_create_technique(gerium_renderer_t renderer,
                                 gerium_frame_graph_t frame_graph,
                                 gerium_utf8_t name,
                                 gerium_uint32_t pipeline_count,
                                 const gerium_pipeline_t* pipelines,
                                 gerium_technique_h* handle);

gerium_public gerium_result_t
gerium_renderer_create_descriptor_set(gerium_renderer_t renderer,
                                      gerium_bool_t global,
                                      gerium_descriptor_set_h* handle);

gerium_public gerium_result_t
gerium_renderer_async_upload_texture_data(gerium_renderer_t renderer,
                                          gerium_texture_h handle,
                                          gerium_cdata_t texture_data,
                                          gerium_texture_loaded_func_t callback,
                                          gerium_data_t data);

gerium_public gerium_result_t
gerium_renderer_texture_sampler(gerium_renderer_t renderer,
                                gerium_texture_h handle,
                                gerium_filter_t min_filter,
                                gerium_filter_t mag_filter,
                                gerium_filter_t mip_filter,
                                gerium_address_mode_t address_mode_u,
                                gerium_address_mode_t address_mode_v,
                                gerium_address_mode_t address_mode_w);

gerium_public void
gerium_renderer_destroy_buffer(gerium_renderer_t renderer,
                               gerium_buffer_h handle);

gerium_public void
gerium_renderer_destroy_texture(gerium_renderer_t renderer,
                                gerium_texture_h handle);

gerium_public void
gerium_renderer_destroy_technique(gerium_renderer_t renderer,
                                  gerium_technique_h handle);

gerium_public void
gerium_renderer_destroy_descriptor_set(gerium_renderer_t renderer,
                                       gerium_descriptor_set_h handle);

gerium_public void
gerium_renderer_bind_buffer(gerium_renderer_t renderer,
                            gerium_descriptor_set_h handle,
                            gerium_uint16_t binding,
                            gerium_buffer_h buffer);

gerium_public void
gerium_renderer_bind_texture(gerium_renderer_t renderer,
                             gerium_descriptor_set_h handle,
                             gerium_uint16_t binding,
                             gerium_uint16_t element,
                             gerium_texture_h texture);

gerium_public void
gerium_renderer_bind_resource(gerium_renderer_t renderer,
                              gerium_descriptor_set_h handle,
                              gerium_uint16_t binding,
                              gerium_utf8_t resource_input);

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
gerium_command_buffer_bind_technique(gerium_command_buffer_t command_buffer,
                                     gerium_technique_h handle);

gerium_public void
gerium_command_buffer_bind_vertex_buffer(gerium_command_buffer_t command_buffer,
                                         gerium_buffer_h handle,
                                         gerium_uint32_t binding,
                                         gerium_uint32_t offset);

gerium_public void
gerium_command_buffer_bind_index_buffer(gerium_command_buffer_t command_buffer,
                                        gerium_buffer_h handle,
                                        gerium_uint32_t offset,
                                        gerium_index_type_t type);

gerium_public void
gerium_command_buffer_bind_descriptor_set(gerium_command_buffer_t command_buffer,
                                          gerium_descriptor_set_h handle,
                                          gerium_uint32_t set);

gerium_public void
gerium_command_buffer_dispatch(gerium_command_buffer_t command_buffer,
                               gerium_uint32_t group_x,
                               gerium_uint32_t group_y,
                               gerium_uint32_t group_z);

gerium_public void
gerium_command_buffer_draw(gerium_command_buffer_t command_buffer,
                           gerium_uint32_t first_vertex,
                           gerium_uint32_t vertex_count,
                           gerium_uint32_t first_instance,
                           gerium_uint32_t instance_count);

gerium_public void
gerium_command_buffer_draw_indexed(gerium_command_buffer_t command_buffer,
                                   gerium_uint32_t first_index,
                                   gerium_uint32_t index_count,
                                   gerium_uint32_t vertex_offset,
                                   gerium_uint32_t first_instance,
                                   gerium_uint32_t instance_count);

gerium_public void
gerium_command_buffer_draw_mesh_tasks(gerium_command_buffer_t command_buffer,
                                      gerium_uint32_t group_x,
                                      gerium_uint32_t group_y,
                                      gerium_uint32_t group_z);

gerium_public void
gerium_command_buffer_draw_mesh_tasks_indirect(gerium_command_buffer_t command_buffer,
                                               gerium_buffer_h handle,
                                               gerium_uint32_t offset,
                                               gerium_uint32_t draw_count,
                                               gerium_uint32_t stride);

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
                            gerium_data_t data);

gerium_public gerium_result_t
gerium_frame_graph_remove_pass(gerium_frame_graph_t frame_graph,
                               gerium_utf8_t name);

gerium_public gerium_result_t
gerium_frame_graph_add_node(gerium_frame_graph_t frame_graph,
                            gerium_utf8_t name,
                            gerium_bool_t compute,
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
