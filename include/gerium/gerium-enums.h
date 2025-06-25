/**
 * @file      gerium-enums.h
 * @brief     Enumerations.
 * @details   Here are declared the enumerations necessary for the work with Gerium.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @copyright MIT License
 */
#ifndef GERIUM_ENUMS_H
#define GERIUM_ENUMS_H

#include "gerium-version.h"
#include "gerium-types.h"

GERIUM_BEGIN

/**
 * @brief   Result codes.
 * @details Status/error codes returned by framework methods.
 */
typedef enum
{
    GERIUM_RESULT_SUCCESS                           = 0, /**< Operation completed successfully. */
    GERIUM_RESULT_SKIP_FRAME                        = 1, /**< Should skip rendering the frame if this code returned ::gerium_renderer_new_frame. */
    GERIUM_RESULT_ERROR_UNKNOWN                     = 2, /**< Unknown error occurred. */
    GERIUM_RESULT_ERROR_OUT_OF_MEMORY               = 3, /**< Memory allocation failed. */
    GERIUM_RESULT_ERROR_NOT_IMPLEMENTED             = 4, /**< Functionality not available. */
    GERIUM_RESULT_ERROR_FILE_OPEN                   = 5, /**< Failed to open file. */
    GERIUM_RESULT_ERROR_FILE_ALLOCATE               = 6, /**< Failed to allocate file space. */
    GERIUM_RESULT_ERROR_FILE_WRITE                  = 7, /**< File write operation failed. */
    GERIUM_RESULT_ERROR_APPLICATION_CREATE          = 8, /**< Application initialization failed. */
    GERIUM_RESULT_ERROR_APPLICATION_ALREADY_RUNNING = 9, /**< Duplicate application instance. */
    GERIUM_RESULT_ERROR_APPLICATION_NOT_RUNNING     = 10, /**< Operation requires running application. */
    GERIUM_RESULT_ERROR_APPLICATION_TERMINATED      = 11, /**< Application was terminated. */
    GERIUM_RESULT_ERROR_NO_DISPLAY                  = 12, /**< No display devices found. */
    GERIUM_RESULT_ERROR_DEVICE_SELECTION            = 13, /**< Failed to select graphics device. */
    GERIUM_RESULT_ERROR_DEVICE_LOST                 = 14, /**< Graphics device became unavailable. */
    GERIUM_RESULT_ERROR_ALREADY_EXISTS              = 15, /**< Resource already exists. */
    GERIUM_RESULT_ERROR_NOT_FOUND                   = 16, /**< Resource not found. */
    GERIUM_RESULT_ERROR_FROM_CALLBACK               = 17, /**< Error originated in callback function. */
    GERIUM_RESULT_ERROR_FEATURE_NOT_SUPPORTED       = 18, /**< Hardware feature unavailable. */
    GERIUM_RESULT_ERROR_FORMAT_NOT_SUPPORTED        = 19, /**< Pixel format not supported. */
    GERIUM_RESULT_ERROR_FIDELITY_FX_NOT_SUPPORTED   = 20, /**< FidelityFX not supported. */
    GERIUM_RESULT_ERROR_INVALID_ARGUMENT            = 21, /**< Invalid parameter passed. */
    GERIUM_RESULT_ERROR_INVALID_FRAME_GRAPH         = 22, /**< Unable to compile frame graph for given nodes. */
    GERIUM_RESULT_ERROR_INVALID_RESOURCE            = 23, /**< Cannot bind external resource because it has a different resource type. */
    GERIUM_RESULT_ERROR_INVALID_OPERATION           = 24, /**< Illegal operation attempted. */
    GERIUM_RESULT_ERROR_PARSE_SPIRV                 = 25, /**< SPIR-V parsing failed. */
    GERIUM_RESULT_ERROR_DETECT_SHADER_LANGUAGE      = 26, /**< Shader language detection failed. */
    GERIUM_RESULT_ERROR_COMPILE_SHADER              = 27, /**< Shader compilation error. */
    GERIUM_RESULT_ERROR_BINDING                     = 28, /**< Resource binding failed. */
    GERIUM_RESULT_ERROR_DESCRIPTOR                  = 29, /**< Descriptor set error. */
    GERIUM_RESULT_ERROR_LOAD_TEXTURE                = 30, /**< Texture loading failed. */
    GERIUM_RESULT_ERROR_CHANGE_DISPLAY_MODE         = 31, /**< Display mode change rejected. */
    GERIUM_RESULT_MAX_ENUM                          = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_result_t;

/**
 * @brief   File seek origins.
 * @details Reference positions for file seek operations.
 */
typedef enum
{
    GERIUM_FILE_SEEK_BEGIN    = 0, /**< Seek from start of file. */
    GERIUM_FILE_SEEK_CURRENT  = 1, /**< Seek from current position. */
    GERIUM_FILE_SEEK_END      = 2, /**< Seek from end of file. */
    GERIUM_FILE_SEEK_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_file_seek_t;

/**
 * @brief   Logging severity levels.
 * @details Hierarchical message importance classification.
 */
typedef enum
{
    GERIUM_LOGGER_LEVEL_VERBOSE  = 0, /**< Detailed diagnostic messages. */
    GERIUM_LOGGER_LEVEL_DEBUG    = 1, /**< Debugging information. */
    GERIUM_LOGGER_LEVEL_INFO     = 2, /**< Informational messages. */
    GERIUM_LOGGER_LEVEL_WARNING  = 3, /**< Potentially problematic situations. */
    GERIUM_LOGGER_LEVEL_ERROR    = 4, /**< Recoverable error conditions. */
    GERIUM_LOGGER_LEVEL_FATAL    = 5, /**< Critical unrecoverable errors. */
    GERIUM_LOGGER_LEVEL_OFF      = 6, /**< No logging output. */
    GERIUM_LOGGER_LEVEL_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_logger_level_t;

/**
 * @brief   Runtime platforms.
 * @details Supported operating systems and execution environments.
 */
typedef enum
{
    GERIUM_RUNTIME_PLATFORM_UNKNOWN  = 0, /**< Undetermined platform. */
    GERIUM_RUNTIME_PLATFORM_ANDROID  = 1, /**< Android OS. */
    GERIUM_RUNTIME_PLATFORM_IOS      = 2, /**< Apple iOS. */
    GERIUM_RUNTIME_PLATFORM_WEB      = 3, /**< Web browser environment. */
    GERIUM_RUNTIME_PLATFORM_WINDOWS  = 4, /**< Microsoft Windows. */
    GERIUM_RUNTIME_PLATFORM_LINUX    = 5, /**< Linux operating system. */
    GERIUM_RUNTIME_PLATFORM_MAC_OS   = 6, /**< Apple macOS. */
    GERIUM_RUNTIME_PLATFORM_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_runtime_platform_t;

/**
 * @brief   Application state changes.
 * @details Lifecycle and visibility states of the application.
 */
typedef enum
{
    GERIUM_APPLICATION_STATE_UNKNOWN      = 0, /**< Unknown state. */
    GERIUM_APPLICATION_STATE_CREATE       = 1, /**< Application creation started. */
    GERIUM_APPLICATION_STATE_DESTROY      = 2, /**< Application destruction started. */
    GERIUM_APPLICATION_STATE_INITIALIZE   = 3, /**< Window initialized. */
    GERIUM_APPLICATION_STATE_UNINITIALIZE = 4, /**< Window shutting down. */
    GERIUM_APPLICATION_STATE_GOT_FOCUS    = 5, /**< Window gained focus. */
    GERIUM_APPLICATION_STATE_LOST_FOCUS   = 6, /**< Window lost focus. */
    GERIUM_APPLICATION_STATE_VISIBLE      = 7, /**< Window became visible on screen. */
    GERIUM_APPLICATION_STATE_INVISIBLE    = 8, /**< Window became hidden/minimized. */
    GERIUM_APPLICATION_STATE_NORMAL       = 9, /**< Restored to normal window state. */
    GERIUM_APPLICATION_STATE_MINIMIZE     = 10, /**< Window minimized. */
    GERIUM_APPLICATION_STATE_MAXIMIZE     = 11, /**< Window maximized. */
    GERIUM_APPLICATION_STATE_FULLSCREEN   = 12, /**< Entered fullscreen mode. */
    GERIUM_APPLICATION_STATE_RESIZE       = 13, /**< Window resize started. */
    GERIUM_APPLICATION_STATE_RESIZED      = 14, /**< Window resize completed. */
    GERIUM_APPLICATION_STATE_MAX_ENUM     = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_application_state_t;

/**
 * @brief   Window style flags.
 * @details Configurable window decoration and behavior options.
 */
typedef enum
{
    GERIUM_APPLICATION_STYLE_NONE_BIT        = 0, /**< No special styling. */
    GERIUM_APPLICATION_STYLE_RESIZABLE_BIT   = 1, /**< Allow window resizing. */
    GERIUM_APPLICATION_STYLE_MINIMIZABLE_BIT = 2, /**< Show minimize button and allow minimize. */
    GERIUM_APPLICATION_STYLE_MAXIMIZABLE_BIT = 4, /**< Show maximize button and allow maximizable. */
    GERIUM_APPLICATION_STYLE_MAX_ENUM        = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_application_style_flags_t;
GERIUM_FLAGS(gerium_application_style_flags_t)

/**
 * @brief   Input event types.
 * @details Categories of user input events.
 */
typedef enum
{
    GERIUM_EVENT_TYPE_KEYBOARD = 0, /**< Key press/release events. */
    GERIUM_EVENT_TYPE_MOUSE    = 1, /**< Mouse movement/button events. */
    GERIUM_EVENT_TYPE_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_event_type_t;

/**
 * @brief   Key state transitions.
 * @details Keyboard key action states.
 */
typedef enum
{
    GERIUM_KEY_STATE_PRESSED  = 0, /**< Key was pressed down. */
    GERIUM_KEY_STATE_RELEASED = 1, /**< Key was released up. */
    GERIUM_KEY_STATE_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_key_state_t;

/**
 * @brief   Keyboard modifier keys.
 * @details Bitmask of active modifier keys.
 */
typedef enum
{
    GERIUM_KEY_MOD_NONE_BIT        = 0x00, /**< No modifiers active. */
    GERIUM_KEY_MOD_LSHIFT_BIT      = 0x01, /**< Left Shift key. */
    GERIUM_KEY_MOD_RSHIFT_BIT      = 0x02, /**< Right Shift key. */
    GERIUM_KEY_MOD_LCTRL_BIT       = 0x04, /**< Left Control key. */
    GERIUM_KEY_MOD_RCTRL_BIT       = 0x08, /**< Right Control key. */
    GERIUM_KEY_MOD_LALT_BIT        = 0x10, /**< Left Alt key. */
    GERIUM_KEY_MOD_RALT_BIT        = 0x20, /**< Right Alt key. */
    GERIUM_KEY_MOD_LMETA_BIT       = 0x40, /**< Left Meta/OS key. */
    GERIUM_KEY_MOD_RMETA_BIT       = 0x80, /**< Right Meta/OS key. */
    GERIUM_KEY_MOD_NUM_LOCK_BIT    = 0x0100, /**< Num Lock enabled. */
    GERIUM_KEY_MOD_CAPS_LOCK_BIT   = 0x0200, /**< Caps Lock enabled. */
    GERIUM_KEY_MOD_SCROLL_LOCK_BIT = 0x0400, /**< Scroll Lock enabled. */
    GERIUM_KEY_MOD_SHIFT_BIT       = GERIUM_KEY_MOD_LSHIFT_BIT | GERIUM_KEY_MOD_RSHIFT_BIT, /**< Either Shift key. */
    GERIUM_KEY_MOD_CTRL_BIT        = GERIUM_KEY_MOD_LCTRL_BIT | GERIUM_KEY_MOD_RCTRL_BIT, /**< Either Control key. */
    GERIUM_KEY_MOD_ALT_BIT         = GERIUM_KEY_MOD_LALT_BIT | GERIUM_KEY_MOD_RALT_BIT, /**< Either Alt key. */
    GERIUM_KEY_MOD_META_BIT        = GERIUM_KEY_MOD_LMETA_BIT | GERIUM_KEY_MOD_RMETA_BIT, /**< Either Meta key. */
    GERIUM_KEY_MOD_MAX_ENUM        = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_key_mod_flags_t;
GERIUM_FLAGS(gerium_key_mod_flags_t)

/**
 * @brief   Keyboard scancodes.
 * @details Physical key identifiers independent of keyboard layout.
 * @note    Scancodes are used for controls where it is important to know which key is pressed,
 *          regardless of the keyboard layout. For example, the game uses the WASD keys to
 *          move, which generate the same scancodes on different layouts.
 */
typedef enum
{
    GERIUM_SCANCODE_UNKNOWN              = 0, /**< Unidentified key. */
    GERIUM_SCANCODE_0                    = 1, /**< 0 key. */
    GERIUM_SCANCODE_1                    = 2, /**< 1 key. */
    GERIUM_SCANCODE_2                    = 3, /**< 2 key. */
    GERIUM_SCANCODE_3                    = 4, /**< 3 key. */
    GERIUM_SCANCODE_4                    = 5, /**< 4 key. */
    GERIUM_SCANCODE_5                    = 6, /**< 5 key. */
    GERIUM_SCANCODE_6                    = 7, /**< 6 key. */
    GERIUM_SCANCODE_7                    = 8, /**< 7 key. */
    GERIUM_SCANCODE_8                    = 9, /**< 8 key. */
    GERIUM_SCANCODE_9                    = 10, /**< 9 key. */
    GERIUM_SCANCODE_A                    = 11, /**< A key. */
    GERIUM_SCANCODE_B                    = 12, /**< B key. */
    GERIUM_SCANCODE_C                    = 13, /**< C key. */
    GERIUM_SCANCODE_D                    = 14, /**< D key. */
    GERIUM_SCANCODE_E                    = 15, /**< E key. */
    GERIUM_SCANCODE_F                    = 16, /**< F key. */
    GERIUM_SCANCODE_G                    = 17, /**< G key. */
    GERIUM_SCANCODE_H                    = 18, /**< H key. */
    GERIUM_SCANCODE_I                    = 19, /**< I key. */
    GERIUM_SCANCODE_J                    = 20, /**< J key. */
    GERIUM_SCANCODE_K                    = 21, /**< K key. */
    GERIUM_SCANCODE_L                    = 22, /**< L key. */
    GERIUM_SCANCODE_M                    = 23, /**< M key. */
    GERIUM_SCANCODE_N                    = 24, /**< N key. */
    GERIUM_SCANCODE_O                    = 25, /**< O key. */
    GERIUM_SCANCODE_P                    = 26, /**< P key. */
    GERIUM_SCANCODE_Q                    = 27, /**< Q key. */
    GERIUM_SCANCODE_R                    = 28, /**< R key. */
    GERIUM_SCANCODE_S                    = 29, /**< S key. */
    GERIUM_SCANCODE_T                    = 30, /**< T key. */
    GERIUM_SCANCODE_U                    = 31, /**< U key. */
    GERIUM_SCANCODE_V                    = 32, /**< V key. */
    GERIUM_SCANCODE_W                    = 33, /**< W key. */
    GERIUM_SCANCODE_X                    = 34, /**< X key. */
    GERIUM_SCANCODE_Y                    = 35, /**< Y key. */
    GERIUM_SCANCODE_Z                    = 36, /**< Z key. */
    GERIUM_SCANCODE_F1                   = 37, /**< F1 function key. */
    GERIUM_SCANCODE_F2                   = 38, /**< F2 function key. */
    GERIUM_SCANCODE_F3                   = 39, /**< F3 function key. */
    GERIUM_SCANCODE_F4                   = 40, /**< F4 function key. */
    GERIUM_SCANCODE_F5                   = 41, /**< F5 function key. */
    GERIUM_SCANCODE_F6                   = 42, /**< F6 function key. */
    GERIUM_SCANCODE_F7                   = 43, /**< F7 function key. */
    GERIUM_SCANCODE_F8                   = 44, /**< F8 function key. */
    GERIUM_SCANCODE_F9                   = 45, /**< F9 function key. */
    GERIUM_SCANCODE_F10                  = 46, /**< F10 function key. */
    GERIUM_SCANCODE_F11                  = 47, /**< F11 function key. */
    GERIUM_SCANCODE_F12                  = 48, /**< F12 function key. */
    GERIUM_SCANCODE_F13                  = 49, /**< F13 function key. */
    GERIUM_SCANCODE_F14                  = 50, /**< F14 function key. */
    GERIUM_SCANCODE_F15                  = 51, /**< F15 function key. */
    GERIUM_SCANCODE_F16                  = 52, /**< F16 function key. */
    GERIUM_SCANCODE_F17                  = 53, /**< F17 function key. */
    GERIUM_SCANCODE_F18                  = 54, /**< F18 function key. */
    GERIUM_SCANCODE_F19                  = 55, /**< F19 function key. */
    GERIUM_SCANCODE_F20                  = 56, /**< F20 function key. */
    GERIUM_SCANCODE_F21                  = 57, /**< F21 function key. */
    GERIUM_SCANCODE_F22                  = 58, /**< F22 function key. */
    GERIUM_SCANCODE_F23                  = 59, /**< F23 function key. */
    GERIUM_SCANCODE_F24                  = 60, /**< F24 function key. */
    GERIUM_SCANCODE_NUMPAD_0             = 61, /**< Numpad 0 key. */
    GERIUM_SCANCODE_NUMPAD_1             = 62, /**< Numpad 1 key. */
    GERIUM_SCANCODE_NUMPAD_2             = 63, /**< Numpad 2 key. */
    GERIUM_SCANCODE_NUMPAD_3             = 64, /**< Numpad 3 key. */
    GERIUM_SCANCODE_NUMPAD_4             = 65, /**< Numpad 4 key. */
    GERIUM_SCANCODE_NUMPAD_5             = 66, /**< Numpad 5 key. */
    GERIUM_SCANCODE_NUMPAD_6             = 67, /**< Numpad 6 key. */
    GERIUM_SCANCODE_NUMPAD_7             = 68, /**< Numpad 7 key. */
    GERIUM_SCANCODE_NUMPAD_8             = 69, /**< Numpad 8 key. */
    GERIUM_SCANCODE_NUMPAD_9             = 70, /**< Numpad 9 key. */
    GERIUM_SCANCODE_NUMPAD_COMMA         = 71, /**< Numpad comma key. */
    GERIUM_SCANCODE_NUMPAD_ENTER         = 72, /**< Numpad enter key. */
    GERIUM_SCANCODE_NUMPAD_EQUAL         = 73, /**< Numpad equals key. */
    GERIUM_SCANCODE_NUMPAD_SUBTRACT      = 74, /**< Numpad subtract key. */
    GERIUM_SCANCODE_NUMPAD_DECIMAL       = 75, /**< Numpad decimal key. */
    GERIUM_SCANCODE_NUMPAD_ADD           = 76, /**< Numpad add key. */
    GERIUM_SCANCODE_NUMPAD_DIVIDE        = 77, /**< Numpad divide key. */
    GERIUM_SCANCODE_NUMPAD_MULTIPLY      = 78, /**< Numpad multiply key. */
    GERIUM_SCANCODE_NUM_LOCK             = 79, /**< Num Lock key. */
    GERIUM_SCANCODE_MEDIA_PLAY_PAUSE     = 80, /**< Media play/pause key. */
    GERIUM_SCANCODE_MEDIA_TRACK_PREVIOUS = 81, /**< Media previous track key. */
    GERIUM_SCANCODE_MEDIA_TRACK_NEXT     = 82, /**< Media next track key. */
    GERIUM_SCANCODE_MEDIA_STOP           = 83, /**< Media stop key. */
    GERIUM_SCANCODE_LAUNCH_MEDIA_PLAYER  = 84, /**< Media player launch key. */
    GERIUM_SCANCODE_AUDIO_VOLUME_MUTE    = 85, /**< Volume mute key. */
    GERIUM_SCANCODE_AUDIO_VOLUME_DOWN    = 86, /**< Volume down key. */
    GERIUM_SCANCODE_AUDIO_VOLUME_UP      = 87, /**< Volume up key. */
    GERIUM_SCANCODE_ESCAPE               = 88, /**< Escape key. */
    GERIUM_SCANCODE_TAB                  = 89, /**< Tab key. */
    GERIUM_SCANCODE_CAPS_LOCK            = 90, /**< Caps Lock key. */
    GERIUM_SCANCODE_ENTER                = 91, /**< Enter key. */
    GERIUM_SCANCODE_BACKSLASH            = 92, /**< Backslash key. */
    GERIUM_SCANCODE_BACKSPACE            = 93, /**< Backspace key. */
    GERIUM_SCANCODE_INTL_BACKSLASH       = 94, /**< International backslash key. */
    GERIUM_SCANCODE_INTL_RO              = 95, /**< Japanese Ro key. */
    GERIUM_SCANCODE_INTL_YEN             = 96, /**< Japanese Yen key. */
    GERIUM_SCANCODE_MINUS                = 97, /**< Minus key. */
    GERIUM_SCANCODE_COLON                = 98, /**< Colon key. */
    GERIUM_SCANCODE_COMMA                = 99, /**< Comma key. */
    GERIUM_SCANCODE_CONVERT              = 100, /**< IME convert key. */
    GERIUM_SCANCODE_NONCONVERT           = 101, /**< IME non-convert key. */
    GERIUM_SCANCODE_EQUAL                = 102, /**< Equals key. */
    GERIUM_SCANCODE_PERIOD               = 103, /**< Period key. */
    GERIUM_SCANCODE_POWER                = 104, /**< Power key. */
    GERIUM_SCANCODE_SEMICOLON            = 105, /**< Semicolon key. */
    GERIUM_SCANCODE_SLASH                = 106, /**< Slash key. */
    GERIUM_SCANCODE_SLEEP                = 107, /**< Sleep key. */
    GERIUM_SCANCODE_WAKE                 = 108, /**< Wake key. */
    GERIUM_SCANCODE_SPACE                = 109, /**< Spacebar. */
    GERIUM_SCANCODE_QUOTE                = 110, /**< Quote key. */
    GERIUM_SCANCODE_BACKQUOTE            = 111, /**< Backquote key. */
    GERIUM_SCANCODE_ALT_LEFT             = 112, /**< Left Alt key. */
    GERIUM_SCANCODE_ALT_RIGHT            = 113, /**< Right Alt key. */
    GERIUM_SCANCODE_BRACKET_LEFT         = 114, /**< Left bracket key. */
    GERIUM_SCANCODE_BRACKET_RIGHT        = 115, /**< Right bracket key. */
    GERIUM_SCANCODE_CONTROL_LEFT         = 116, /**< Left Control key. */
    GERIUM_SCANCODE_CONTROL_RIGHT        = 117, /**< Right Control key. */
    GERIUM_SCANCODE_SHIFT_LEFT           = 118, /**< Left Shift key. */
    GERIUM_SCANCODE_SHIFT_RIGHT          = 119, /**< Right Shift key. */
    GERIUM_SCANCODE_META_LEFT            = 120, /**< Left Meta/OS key. */
    GERIUM_SCANCODE_META_RIGHT           = 121, /**< Right Meta/OS key. */
    GERIUM_SCANCODE_ARROW_UP             = 122, /**< Up arrow key. */
    GERIUM_SCANCODE_ARROW_LEFT           = 123, /**< Left arrow key. */
    GERIUM_SCANCODE_ARROW_RIGHT          = 124, /**< Right arrow key. */
    GERIUM_SCANCODE_ARROW_DOWN           = 125, /**< Down arrow key. */
    GERIUM_SCANCODE_SCROLL_LOCK          = 126, /**< Scroll Lock key. */
    GERIUM_SCANCODE_PAUSE                = 127, /**< Pause key. */
    GERIUM_SCANCODE_CTRL_PAUSE           = 128, /**< Ctrl+Pause key combination. */
    GERIUM_SCANCODE_INSERT               = 129, /**< Insert key. */
    GERIUM_SCANCODE_DELETE               = 130, /**< Delete key. */
    GERIUM_SCANCODE_HOME                 = 131, /**< Home key. */
    GERIUM_SCANCODE_END                  = 132, /**< End key. */
    GERIUM_SCANCODE_PAGE_UP              = 133, /**< Page Up key. */
    GERIUM_SCANCODE_PAGE_DOWN            = 134, /**< Page Down key. */
    GERIUM_SCANCODE_LAUNCH_MAIL          = 135, /**< Mail launch key. */
    GERIUM_SCANCODE_MYCOMPUTER           = 136, /**< My Computer key. */
    GERIUM_SCANCODE_CONTEXT_MENU         = 137, /**< Context Menu key. */
    GERIUM_SCANCODE_PRINT_SCREEN         = 138, /**< Print Screen key. */
    GERIUM_SCANCODE_ALT_PRINT_SCREEN     = 139, /**< Alt+PrintScreen combination. */
    GERIUM_SCANCODE_LAUNCH_APPLICATION_1 = 140, /**< Application 1 launch key. */
    GERIUM_SCANCODE_LAUNCH_APPLICATION_2 = 141, /**< Application 2 launch key. */
    GERIUM_SCANCODE_KANA_MODE            = 142, /**< Japanese Kana mode key. */
    GERIUM_SCANCODE_BROWSER_BACK         = 143, /**< Browser back key. */
    GERIUM_SCANCODE_BROWSER_FAVORITES    = 144, /**< Browser favorites key. */
    GERIUM_SCANCODE_BROWSER_FORWARD      = 145, /**< Browser forward key. */
    GERIUM_SCANCODE_BROWSER_HOME         = 146, /**< Browser home key. */
    GERIUM_SCANCODE_BROWSER_REFRESH      = 147, /**< Browser refresh key. */
    GERIUM_SCANCODE_BROWSER_SEARCH       = 148, /**< Browser search key. */
    GERIUM_SCANCODE_BROWSER_STOP         = 149, /**< Browser stop key. */
    GERIUM_SCANCODE_MAX_ENUM             = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_scancode_t;

/**
 * @brief   Keyboard key codes.
 * @details Logical key identifiers representing actual characters.
 * @note    Keycode determines what function a key performs in the context of the current keyboard layout.
 */
typedef enum
{
    GERIUM_KEY_CODE_UNKNOWN              = 0, /**< Unidentified key. */
    GERIUM_KEY_CODE_0                    = 1, /**< 0 key. */
    GERIUM_KEY_CODE_1                    = 2, /**< 1 key. */
    GERIUM_KEY_CODE_2                    = 3, /**< 2 key. */
    GERIUM_KEY_CODE_3                    = 4, /**< 3 key. */
    GERIUM_KEY_CODE_4                    = 5, /**< 4 key. */
    GERIUM_KEY_CODE_5                    = 6, /**< 5 key. */
    GERIUM_KEY_CODE_6                    = 7, /**< 6 key. */
    GERIUM_KEY_CODE_7                    = 8, /**< 7 key. */
    GERIUM_KEY_CODE_8                    = 9, /**< 8 key. */
    GERIUM_KEY_CODE_9                    = 10, /**< 9 key. */
    GERIUM_KEY_CODE_A                    = 11, /**< A key. */
    GERIUM_KEY_CODE_B                    = 12, /**< B key. */
    GERIUM_KEY_CODE_C                    = 13, /**< C key. */
    GERIUM_KEY_CODE_D                    = 14, /**< D key. */
    GERIUM_KEY_CODE_E                    = 15, /**< E key. */
    GERIUM_KEY_CODE_F                    = 16, /**< F key. */
    GERIUM_KEY_CODE_G                    = 17, /**< G key. */
    GERIUM_KEY_CODE_H                    = 18, /**< H key. */
    GERIUM_KEY_CODE_I                    = 19, /**< I key. */
    GERIUM_KEY_CODE_J                    = 20, /**< J key. */
    GERIUM_KEY_CODE_K                    = 21, /**< K key. */
    GERIUM_KEY_CODE_L                    = 22, /**< L key. */
    GERIUM_KEY_CODE_M                    = 23, /**< M key. */
    GERIUM_KEY_CODE_N                    = 24, /**< N key. */
    GERIUM_KEY_CODE_O                    = 25, /**< O key. */
    GERIUM_KEY_CODE_P                    = 26, /**< P key. */
    GERIUM_KEY_CODE_Q                    = 27, /**< Q key. */
    GERIUM_KEY_CODE_R                    = 28, /**< R key. */
    GERIUM_KEY_CODE_S                    = 29, /**< S key. */
    GERIUM_KEY_CODE_T                    = 30, /**< T key. */
    GERIUM_KEY_CODE_U                    = 31, /**< U key. */
    GERIUM_KEY_CODE_V                    = 32, /**< V key. */
    GERIUM_KEY_CODE_W                    = 33, /**< W key. */
    GERIUM_KEY_CODE_X                    = 34, /**< X key. */
    GERIUM_KEY_CODE_Y                    = 35, /**< Y key. */
    GERIUM_KEY_CODE_Z                    = 36, /**< Z key. */
    GERIUM_KEY_CODE_F1                   = 37, /**< F1 function key. */
    GERIUM_KEY_CODE_F2                   = 38, /**< F2 function key. */
    GERIUM_KEY_CODE_F3                   = 39, /**< F3 function key. */
    GERIUM_KEY_CODE_F4                   = 40, /**< F4 function key. */
    GERIUM_KEY_CODE_F5                   = 41, /**< F5 function key. */
    GERIUM_KEY_CODE_F6                   = 42, /**< F6 function key. */
    GERIUM_KEY_CODE_F7                   = 43, /**< F7 function key. */
    GERIUM_KEY_CODE_F8                   = 44, /**< F8 function key. */
    GERIUM_KEY_CODE_F9                   = 45, /**< F9 function key. */
    GERIUM_KEY_CODE_F10                  = 46, /**< F10 function key. */
    GERIUM_KEY_CODE_F11                  = 47, /**< F11 function key. */
    GERIUM_KEY_CODE_F12                  = 48, /**< F12 function key. */
    GERIUM_KEY_CODE_F13                  = 49, /**< F13 function key. */
    GERIUM_KEY_CODE_F14                  = 50, /**< F14 function key. */
    GERIUM_KEY_CODE_F15                  = 51, /**< F15 function key. */
    GERIUM_KEY_CODE_F16                  = 52, /**< F16 function key. */
    GERIUM_KEY_CODE_F17                  = 53, /**< F17 function key. */
    GERIUM_KEY_CODE_F18                  = 54, /**< F18 function key. */
    GERIUM_KEY_CODE_F19                  = 55, /**< F19 function key. */
    GERIUM_KEY_CODE_F20                  = 56, /**< F20 function key. */
    GERIUM_KEY_CODE_F21                  = 57, /**< F21 function key. */
    GERIUM_KEY_CODE_F22                  = 58, /**< F22 function key. */
    GERIUM_KEY_CODE_F23                  = 59, /**< F23 function key. */
    GERIUM_KEY_CODE_F24                  = 60, /**< F24 function key. */
    GERIUM_KEY_CODE_EXCLAIM              = 61, /**< Exclamation mark (`!`). */
    GERIUM_KEY_CODE_AT                   = 62, /**< At symbol (`@`). */
    GERIUM_KEY_CODE_HASH                 = 63, /**< Hash/pound (`#`). */
    GERIUM_KEY_CODE_DOLLAR               = 64, /**< Dollar sign (`$`). */
    GERIUM_KEY_CODE_PERCENT              = 65, /**< Percent sign (`%`). */
    GERIUM_KEY_CODE_CARET                = 66, /**< Caret (`^`). */
    GERIUM_KEY_CODE_AMPERSAND            = 67, /**< Ampersand (`&`). */
    GERIUM_KEY_CODE_ASTERISK             = 68, /**< Asterisk (`*`). */
    GERIUM_KEY_CODE_PAREN_LEFT           = 69, /**< Left parenthesis (`(`). */
    GERIUM_KEY_CODE_PAREN_RIGHT          = 70, /**< Right parenthesis (`)`). */
    GERIUM_KEY_CODE_UNDERSCORE           = 71, /**< Underscore (`_`). */
    GERIUM_KEY_CODE_NUMPAD_0             = 72, /**< Numpad 0. */
    GERIUM_KEY_CODE_NUMPAD_1             = 73, /**< Numpad 1. */
    GERIUM_KEY_CODE_NUMPAD_2             = 74, /**< Numpad 2. */
    GERIUM_KEY_CODE_NUMPAD_3             = 75, /**< Numpad 3. */
    GERIUM_KEY_CODE_NUMPAD_4             = 76, /**< Numpad 4. */
    GERIUM_KEY_CODE_NUMPAD_5             = 77, /**< Numpad 5. */
    GERIUM_KEY_CODE_NUMPAD_6             = 78, /**< Numpad 6. */
    GERIUM_KEY_CODE_NUMPAD_7             = 79, /**< Numpad 7. */
    GERIUM_KEY_CODE_NUMPAD_8             = 80, /**< Numpad 8. */
    GERIUM_KEY_CODE_NUMPAD_9             = 81, /**< Numpad 9. */
    GERIUM_KEY_CODE_DECIMAL              = 82, /**< Decimal point. */
    GERIUM_KEY_CODE_SUBTRACT             = 83, /**< Subtract/minus. */
    GERIUM_KEY_CODE_ADD                  = 84, /**< Add/plus. */
    GERIUM_KEY_CODE_DIVIDE               = 85, /**< Divide. */
    GERIUM_KEY_CODE_MULTIPLY             = 86, /**< Multiply. */
    GERIUM_KEY_CODE_EQUAL                = 87, /**< Equals sign. */
    GERIUM_KEY_CODE_LESS                 = 88, /**< Less-than (<). */
    GERIUM_KEY_CODE_GREATER              = 89, /**< Greater-than (>). */
    GERIUM_KEY_CODE_NUM_LOCK             = 90, /**< Num Lock. */
    GERIUM_KEY_CODE_MEDIA_PLAY_PAUSE     = 91, /**< Media play/pause. */
    GERIUM_KEY_CODE_MEDIA_TRACK_PREVIOUS = 92, /**< Previous track. */
    GERIUM_KEY_CODE_MEDIA_TRACK_NEXT     = 93, /**< Next track. */
    GERIUM_KEY_CODE_MEDIA_STOP           = 94, /**< Media stop. */
    GERIUM_KEY_CODE_LAUNCH_MEDIA_PLAYER  = 95, /**< Media player. */
    GERIUM_KEY_CODE_AUDIO_VOLUME_MUTE    = 96, /**< Volume mute. */
    GERIUM_KEY_CODE_AUDIO_VOLUME_DOWN    = 97, /**< Volume down. */
    GERIUM_KEY_CODE_AUDIO_VOLUME_UP      = 98, /**< Volume up. */
    GERIUM_KEY_CODE_ESCAPE               = 99, /**< Escape. */
    GERIUM_KEY_CODE_TAB                  = 100, /**< Tab. */
    GERIUM_KEY_CODE_CAPS_LOCK            = 101, /**< Caps Lock. */
    GERIUM_KEY_CODE_ENTER                = 102, /**< Enter. */
    GERIUM_KEY_CODE_BACKSLASH            = 103, /**< Backslash (`\`). */
    GERIUM_KEY_CODE_PIPE                 = 104, /**< Pipe (`|`). */
    GERIUM_KEY_CODE_QUESTION             = 105, /**< Question mark (`?`). */
    GERIUM_KEY_CODE_BACKSPACE            = 106, /**< Backspace. */
    GERIUM_KEY_CODE_COLON                = 107, /**< Colon (`:`). */
    GERIUM_KEY_CODE_COMMA                = 108, /**< Comma (`,`). */
    GERIUM_KEY_CODE_CONVERT              = 109, /**< IME convert. */
    GERIUM_KEY_CODE_NONCONVERT           = 110, /**< IME non-convert. */
    GERIUM_KEY_CODE_PERIOD               = 111, /**< Period (.). */
    GERIUM_KEY_CODE_POWER                = 112, /**< Power. */
    GERIUM_KEY_CODE_SEMICOLON            = 113, /**< Semicolon (`;`). */
    GERIUM_KEY_CODE_SLASH                = 114, /**< Slash (`/`). */
    GERIUM_KEY_CODE_SLEEP                = 115, /**< Sleep. */
    GERIUM_KEY_CODE_WAKE                 = 116, /**< Wake. */
    GERIUM_KEY_CODE_SPACE                = 117, /**< Space. */
    GERIUM_KEY_CODE_DOUBLE_QUOTE         = 118, /**< Double quote (`"`). */
    GERIUM_KEY_CODE_QUOTE                = 119, /**< Single quote ('). */
    GERIUM_KEY_CODE_BACKQUOTE            = 120, /**< Backquote (`). */
    GERIUM_KEY_CODE_TILDE                = 121, /**< Tilde (`~`). */
    GERIUM_KEY_CODE_ALT_LEFT             = 122, /**< Left Alt. */
    GERIUM_KEY_CODE_ALT_RIGHT            = 123, /**< Right Alt. */
    GERIUM_KEY_CODE_BRACKET_LEFT         = 124, /**< Left bracket (`[`). */
    GERIUM_KEY_CODE_BRACKET_RIGHT        = 125, /**< Right bracket (`]`). */
    GERIUM_KEY_CODE_BRACE_LEFT           = 126, /**< Left brace (`{`). */
    GERIUM_KEY_CODE_BRACE_RIGHT          = 127, /**< Right brace (`}`). */
    GERIUM_KEY_CODE_CONTROL_LEFT         = 128, /**< Left Control. */
    GERIUM_KEY_CODE_CONTROL_RIGHT        = 129, /**< Right Control. */
    GERIUM_KEY_CODE_SHIFT_LEFT           = 130, /**< Left Shift. */
    GERIUM_KEY_CODE_SHIFT_RIGHT          = 131, /**< Right Shift. */
    GERIUM_KEY_CODE_META_LEFT            = 132, /**< Left Meta/OS. */
    GERIUM_KEY_CODE_META_RIGHT           = 133, /**< Right Meta/OS. */
    GERIUM_KEY_CODE_ARROW_UP             = 134, /**< Up arrow. */
    GERIUM_KEY_CODE_ARROW_LEFT           = 135, /**< Left arrow. */
    GERIUM_KEY_CODE_ARROW_RIGHT          = 136, /**< Right arrow. */
    GERIUM_KEY_CODE_ARROW_DOWN           = 137, /**< Down arrow. */
    GERIUM_KEY_CODE_SCROLL_LOCK          = 138, /**< Scroll Lock. */
    GERIUM_KEY_CODE_PAUSE                = 139, /**< Pause. */
    GERIUM_KEY_CODE_INSERT               = 140, /**< Insert. */
    GERIUM_KEY_CODE_DELETE               = 141, /**< Delete. */
    GERIUM_KEY_CODE_HOME                 = 142, /**< Home. */
    GERIUM_KEY_CODE_END                  = 143, /**< End. */
    GERIUM_KEY_CODE_PAGE_UP              = 144, /**< Page Up. */
    GERIUM_KEY_CODE_PAGE_DOWN            = 145, /**< Page Down. */
    GERIUM_KEY_CODE_LAUNCH_MAIL          = 146, /**< Mail. */
    GERIUM_KEY_CODE_CONTEXT_MENU         = 147, /**< Context menu. */
    GERIUM_KEY_CODE_PRINT_SCREEN         = 148, /**< Print Screen. */
    GERIUM_KEY_CODE_LAUNCH_APPLICATION_1 = 149, /**< Application 1. */
    GERIUM_KEY_CODE_LAUNCH_APPLICATION_2 = 150, /**< Application 2. */
    GERIUM_KEY_CODE_KANA_MODE            = 151, /**< Kana mode. */
    GERIUM_KEY_CODE_BROWSER_BACK         = 152, /**< Browser back. */
    GERIUM_KEY_CODE_BROWSER_FAVORITES    = 153, /**< Browser favorites. */
    GERIUM_KEY_CODE_BROWSER_FORWARD      = 154, /**< Browser forward. */
    GERIUM_KEY_CODE_BROWSER_HOME         = 155, /**< Browser home. */
    GERIUM_KEY_CODE_BROWSER_REFRESH      = 156, /**< Browser refresh. */
    GERIUM_KEY_CODE_BROWSER_SEARCH       = 157, /**< Browser search. */
    GERIUM_KEY_CODE_BROWSER_STOP         = 158, /**< Browser stop. */
    GERIUM_KEY_CODE_MAX_ENUM             = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_key_code_t;

/**
 * @brief   Mouse button states.
 * @details Bitmask representing mouse button press/release events.
 */
typedef enum
{
    GERIUM_MOUSE_BUTTON_NONE_BIT        = 0x00, /**< No buttons active. */
    GERIUM_MOUSE_BUTTON_LEFT_DOWN_BIT   = 0x01, /**< Left button pressed. */
    GERIUM_MOUSE_BUTTON_LEFT_UP_BIT     = 0x02, /**< Left button released. */
    GERIUM_MOUSE_BUTTON_RIGHT_DOWN_BIT  = 0x04, /**< Right button pressed. */
    GERIUM_MOUSE_BUTTON_RIGHT_UP_BIT    = 0x08, /**< Right button released. */
    GERIUM_MOUSE_BUTTON_MIDDLE_DOWN_BIT = 0x10, /**< Middle button pressed. */
    GERIUM_MOUSE_BUTTON_MIDDLE_UP_BIT   = 0x20, /**< Middle button released. */
    GERIUM_MOUSE_BUTTON_4_DOWN_BIT      = 0x40, /**< Extra button 4 pressed. */
    GERIUM_MOUSE_BUTTON_4_UP_BIT        = 0x80, /**< Extra button 4 released. */
    GERIUM_MOUSE_BUTTON_5_DOWN_BIT      = 0x0100, /**< Extra button 5 pressed. */
    GERIUM_MOUSE_BUTTON_5_UP_BIT        = 0x0200, /**< Extra button 5 released. */
    GERIUM_MOUSE_BUTTON_MAX_ENUM        = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_mouse_button_flags_t;
GERIUM_FLAGS(gerium_mouse_button_flags_t)

/**
 * @brief   Measurement units.
 * @details Supported dimensional units for layout calculations.
 */
typedef enum
{
    GERIUM_DIMENSION_UNIT_PX       = 0, /**< Physical pixels (device-dependent) */
    GERIUM_DIMENSION_UNIT_MM       = 1, /**< Millimeters (physical size) */
    GERIUM_DIMENSION_UNIT_DIP      = 2, /**< Density-independent pixels (logical size) */
    GERIUM_DIMENSION_UNIT_SP       = 3, /**< Scale-independent pixels (for text) */
    GERIUM_DIMENSION_UNIT_PT       = 4, /**< Points (1/72 inch) */
    GERIUM_DIMENSION_UNIT_IN       = 5, /**< Inches (physical size) */
    GERIUM_DIMENSION_UNIT_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_dimension_unit_t;

/**
 * @brief   Graphics features.
 * @details Bitmask of supported hardware capabilities.
 * @sa      Features can be requested when creating a renderer object ::gerium_renderer_create.
 */
typedef enum
{
    GERIUM_FEATURE_NONE_BIT                  = 0x00, /**< No special features */
    GERIUM_FEATURE_BINDLESS_BIT              = 0x01, /**< Bindless resource access */
    GERIUM_FEATURE_GEOMETRY_SHADER_BIT       = 0x02, /**< Geometry shader support */
    GERIUM_FEATURE_MESH_SHADER_BIT           = 0x04, /**< Mesh shader support */
    GERIUM_FEATURE_SAMPLER_FILTER_MINMAX_BIT = 0x08, /**< Min/max sampler filtering */
    GERIUM_FEATURE_DRAW_INDIRECT_BIT         = 0x10, /**< Indirect drawing */
    GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT   = 0x20, /**< Indirect count drawing */
    GERIUM_FEATURE_8_BIT_STORAGE_BIT         = 0x40, /**< 8-bit storage support */
    GERIUM_FEATURE_16_BIT_STORAGE_BIT        = 0x80, /**< 16-bit storage support */
    GERIUM_FEATURE_MAX_ENUM                  = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_feature_flags_t;
GERIUM_FLAGS(gerium_feature_flags_t)

/**
 * @brief   Texture formats.
 * @details Pixel formats for textures.
 */
typedef enum
{
    GERIUM_FORMAT_R4G4_UNORM           = 0, /**< 8-bit packed unsigned normalized format that has a 4-bit R component in bits 4..7, and a 4-bit G component in bits 0..3. */
    GERIUM_FORMAT_R4G4B4A4_UNORM       = 1, /**< 16-bit packed unsigned normalized format that has a 4-bit R component in bits 12..15, a 4-bit G component in bits 8..11, a 4-bit B component in bits 4..7, and a 4-bit A component in bits 0..3. */
    GERIUM_FORMAT_R5G5B5A1_UNORM       = 2, /**< 16-bit packed unsigned normalized format that has a 5-bit R component in bits 11..15, a 5-bit G component in bits 6..10, a 5-bit B component in bits 1..5, and a 1-bit A component in bit 0. */
    GERIUM_FORMAT_R5G6B5_UNORM         = 3, /**< 16-bit packed unsigned normalized format that has a 5-bit R component in bits 11..15, a 6-bit G component in bits 5..10, and a 5-bit B component in bits 0..4. */
    GERIUM_FORMAT_R8_UNORM             = 4, /**< 8-bit unsigned normalized format that has a single 8-bit R component. */
    GERIUM_FORMAT_R8_SNORM             = 5, /**< 8-bit signed normalized format that has a single 8-bit R component. */
    GERIUM_FORMAT_R8_UINT              = 6, /**< 8-bit unsigned integer format that has a single 8-bit R component. */
    GERIUM_FORMAT_R8_SINT              = 7, /**< 8-bit signed integer format that has a single 8-bit R component. */
    GERIUM_FORMAT_R8_SRGB              = 8, /**< 8-bit unsigned normalized format that has a single 8-bit R component stored with sRGB nonlinear encoding. */
    GERIUM_FORMAT_R8_USCALED           = 9, /**< 8-bit unsigned scaled integer format that has a single 8-bit R component. */
    GERIUM_FORMAT_R8_SSCALED           = 10, /**< 8-bit signed scaled integer format that has a single 8-bit R component. */
    GERIUM_FORMAT_R8G8_UNORM           = 11, /**< 16-bit unsigned normalized format that has an 8-bit R component in byte 0, and an 8-bit G component in byte 1. */
    GERIUM_FORMAT_R8G8_SNORM           = 12, /**< 16-bit signed normalized format that has an 8-bit R component in byte 0, and an 8-bit G component in byte 1. */
    GERIUM_FORMAT_R8G8_UINT            = 13, /**< 16-bit unsigned integer format that has an 8-bit R component in byte 0, and an 8-bit G component in byte 1. */
    GERIUM_FORMAT_R8G8_SINT            = 14, /**< 16-bit signed integer format that has an 8-bit R component in byte 0, and an 8-bit G component in byte 1. */
    GERIUM_FORMAT_R8G8_SRGB            = 15, /**< 16-bit unsigned normalized format that has an 8-bit R component stored with sRGB nonlinear encoding in byte 0, and an 8-bit G component stored with sRGB nonlinear encoding in byte 1. */
    GERIUM_FORMAT_R8G8_USCALED         = 16, /**< 16-bit unsigned scaled integer format that has an 8-bit R component in byte 0, and an 8-bit G component in byte 1. */
    GERIUM_FORMAT_R8G8_SSCALED         = 17, /**< 16-bit signed scaled integer format that has an 8-bit R component in byte 0, and an 8-bit G component in byte 1. */
    GERIUM_FORMAT_R8G8B8_UNORM         = 18, /**< 24-bit unsigned normalized format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, and an 8-bit B component in byte 2. */
    GERIUM_FORMAT_R8G8B8_SNORM         = 19, /**< 24-bit signed normalized format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, and an 8-bit B component in byte 2. */
    GERIUM_FORMAT_R8G8B8_UINT          = 20, /**< 24-bit unsigned integer format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, and an 8-bit B component in byte 2. */
    GERIUM_FORMAT_R8G8B8_SINT          = 21, /**< 24-bit signed integer format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, and an 8-bit B component in byte 2. */
    GERIUM_FORMAT_R8G8B8_SRGB          = 22, /**< 24-bit unsigned normalized format that has an 8-bit R component stored with sRGB nonlinear encoding in byte 0, an 8-bit G component stored with sRGB nonlinear encoding in byte 1, and an 8-bit B component stored with sRGB nonlinear encoding in byte 2. */
    GERIUM_FORMAT_R8G8B8_USCALED       = 23, /**< 24-bit unsigned scaled format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, and an 8-bit B component in byte 2. */
    GERIUM_FORMAT_R8G8B8_SSCALED       = 24, /**< 24-bit signed scaled format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, and an 8-bit B component in byte 2. */
    GERIUM_FORMAT_R8G8B8A8_UNORM       = 25, /**< 32-bit unsigned normalized format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, an 8-bit B component in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_R8G8B8A8_SNORM       = 26, /**< 32-bit signed normalized format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, an 8-bit B component in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_R8G8B8A8_UINT        = 27, /**< 32-bit unsigned integer format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, an 8-bit B component in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_R8G8B8A8_SINT        = 28, /**< 32-bit signed integer format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, an 8-bit B component in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_R8G8B8A8_SRGB        = 29, /**< 32-bit unsigned normalized format that has an 8-bit R component stored with sRGB nonlinear encoding in byte 0, an 8-bit G component stored with sRGB nonlinear encoding in byte 1, an 8-bit B component stored with sRGB nonlinear encoding in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_R8G8B8A8_USCALED     = 30, /**< 32-bit unsigned scaled format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, an 8-bit B component in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_R8G8B8A8_SSCALED     = 31, /**< 32-bit signed scaled format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, an 8-bit B component in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_R16_UNORM            = 32, /**< 16-bit unsigned normalized format that has a single 16-bit R component. */
    GERIUM_FORMAT_R16_SNORM            = 33, /**< 16-bit signed normalized format that has a single 16-bit R component. */
    GERIUM_FORMAT_R16_UINT             = 34, /**< 16-bit unsigned integer format that has a single 16-bit R component. */
    GERIUM_FORMAT_R16_SINT             = 35, /**< 16-bit signed integer format that has a single 16-bit R component. */
    GERIUM_FORMAT_R16_SFLOAT           = 36, /**< 16-bit signed floating-point format that has a single 16-bit R component. */
    GERIUM_FORMAT_R16_USCALED          = 37, /**< 16-bit unsigned scaled integer format that has a single 16-bit R component. */
    GERIUM_FORMAT_R16_SSCALED          = 38, /**< 16-bit signed scaled integer format that has a single 16-bit R component. */
    GERIUM_FORMAT_R16G16_UNORM         = 39, /**< 32-bit unsigned normalized format that has a 16-bit R component in bytes 0..1, and a 16-bit G component in bytes 2..3. */
    GERIUM_FORMAT_R16G16_SNORM         = 40, /**< 32-bit signed normalized format that has a 16-bit R component in bytes 0..1, and a 16-bit G component in bytes 2..3. */
    GERIUM_FORMAT_R16G16_UINT          = 41, /**< 32-bit unsigned integer format that has a 16-bit R component in bytes 0..1, and a 16-bit G component in bytes 2..3. */
    GERIUM_FORMAT_R16G16_SINT          = 42, /**< 32-bit signed integer format that has a 16-bit R component in bytes 0..1, and a 16-bit G component in bytes 2..3. */
    GERIUM_FORMAT_R16G16_SFLOAT        = 43, /**< 32-bit signed floating-point format that has a 16-bit R component in bytes 0..1, and a 16-bit G component in bytes 2..3. */
    GERIUM_FORMAT_R16G16_USCALED       = 44, /**< 32-bit unsigned scaled integer format that has a 16-bit R component in bytes 0..1, and a 16-bit G component in bytes 2..3. */
    GERIUM_FORMAT_R16G16_SSCALED       = 45, /**< 32-bit signed scaled integer format that has a 16-bit R component in bytes 0..1, and a 16-bit G component in bytes 2..3. */
    GERIUM_FORMAT_R16G16B16_UNORM      = 46, /**< 48-bit unsigned normalized format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, and a 16-bit B component in bytes 4..5. */
    GERIUM_FORMAT_R16G16B16_SNORM      = 47, /**< 48-bit signed normalized format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, and a 16-bit B component in bytes 4..5. */
    GERIUM_FORMAT_R16G16B16_UINT       = 48, /**< 48-bit unsigned integer format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, and a 16-bit B component in bytes 4..5. */
    GERIUM_FORMAT_R16G16B16_SINT       = 49, /**< 48-bit signed integer format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, and a 16-bit B component in bytes 4..5. */
    GERIUM_FORMAT_R16G16B16_SFLOAT     = 50, /**< 48-bit signed floating-point format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, and a 16-bit B component in bytes 4..5. */
    GERIUM_FORMAT_R16G16B16_USCALED    = 51, /**< 48-bit unsigned scaled integer format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, and a 16-bit B component in bytes 4..5. */
    GERIUM_FORMAT_R16G16B16_SSCALED    = 52, /**< 48-bit signed scaled integer format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, and a 16-bit B component in bytes 4..5. */
    GERIUM_FORMAT_R16G16B16A16_UNORM   = 53, /**< 64-bit unsigned normalized format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, a 16-bit B component in bytes 4..5, and a 16-bit A component in bytes 6..7. */
    GERIUM_FORMAT_R16G16B16A16_SNORM   = 54, /**< 64-bit signed normalized format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, a 16-bit B component in bytes 4..5, and a 16-bit A component in bytes 6..7. */
    GERIUM_FORMAT_R16G16B16A16_UINT    = 55, /**< 64-bit unsigned integer format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, a 16-bit B component in bytes 4..5, and a 16-bit A component in bytes 6..7. */
    GERIUM_FORMAT_R16G16B16A16_SINT    = 56, /**< 64-bit signed integer format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, a 16-bit B component in bytes 4..5, and a 16-bit A component in bytes 6..7. */
    GERIUM_FORMAT_R16G16B16A16_SFLOAT  = 57, /**< 64-bit signed floating-point format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, a 16-bit B component in bytes 4..5, and a 16-bit A component in bytes 6..7. */
    GERIUM_FORMAT_R16G16B16A16_USCALED = 58, /**< 64-bit unsigned scaled integer format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, a 16-bit B component in bytes 4..5, and a 16-bit A component in bytes 6..7. */
    GERIUM_FORMAT_R16G16B16A16_SSCALED = 59, /**< 64-bit signed scaled integer format that has a 16-bit R component in bytes 0..1, a 16-bit G component in bytes 2..3, a 16-bit B component in bytes 4..5, and a 16-bit A component in bytes 6..7. */
    GERIUM_FORMAT_R32_UINT             = 60, /**< 32-bit unsigned integer format that has a single 32-bit R component. */
    GERIUM_FORMAT_R32_SINT             = 61, /**< 32-bit signed integer format that has a single 32-bit R component. */
    GERIUM_FORMAT_R32_SFLOAT           = 62, /**< 32-bit signed floating-point format that has a single 32-bit R component. */
    GERIUM_FORMAT_R32G32_UINT          = 63, /**< 64-bit unsigned integer format that has a 32-bit R component in bytes 0..3, and a 32-bit G component in bytes 4..7. */
    GERIUM_FORMAT_R32G32_SINT          = 64, /**< 64-bit signed integer format that has a 32-bit R component in bytes 0..3, and a 32-bit G component in bytes 4..7. */
    GERIUM_FORMAT_R32G32_SFLOAT        = 65, /**< 64-bit signed floating-point format that has a 32-bit R component in bytes 0..3, and a 32-bit G component in bytes 4..7. */
    GERIUM_FORMAT_R32G32B32_UINT       = 66, /**< 96-bit unsigned integer format that has a 32-bit R component in bytes 0..3, a 32-bit G component in bytes 4..7, and a 32-bit B component in bytes 8..11. */
    GERIUM_FORMAT_R32G32B32_SINT       = 67, /**< 96-bit signed integer format that has a 32-bit R component in bytes 0..3, a 32-bit G component in bytes 4..7, and a 32-bit B component in bytes 8..11. */
    GERIUM_FORMAT_R32G32B32_SFLOAT     = 68, /**< 96-bit signed floating-point format that has a 32-bit R component in bytes 0..3, a 32-bit G component in bytes 4..7, and a 32-bit B component in bytes 8..11. */
    GERIUM_FORMAT_R32G32B32A32_UINT    = 69, /**< 128-bit unsigned integer format that has a 32-bit R component in bytes 0..3, a 32-bit G component in bytes 4..7, a 32-bit B component in bytes 8..11, and a 32-bit A component in bytes 12..15. */
    GERIUM_FORMAT_R32G32B32A32_SINT    = 70, /**< 128-bit signed integer format that has a 32-bit R component in bytes 0..3, a 32-bit G component in bytes 4..7, a 32-bit B component in bytes 8..11, and a 32-bit A component in bytes 12..15. */
    GERIUM_FORMAT_R32G32B32A32_SFLOAT  = 71, /**< 128-bit signed floating-point format that has a 32-bit R component in bytes 0..3, a 32-bit G component in bytes 4..7, a 32-bit B component in bytes 8..11, and a 32-bit A component in bytes 12..15. */
    GERIUM_FORMAT_R64_UINT             = 72, /**< 64-bit unsigned integer format that has a single 64-bit R component. */
    GERIUM_FORMAT_R64_SINT             = 73, /**< 64-bit signed integer format that has a single 64-bit R component. */
    GERIUM_FORMAT_R64_SFLOAT           = 74, /**< 64-bit signed floating-point format that has a single 64-bit R component. */
    GERIUM_FORMAT_R64G64_UINT          = 75, /**< 128-bit unsigned integer format that has a 64-bit R component in bytes 0..7, and a 64-bit G component in bytes 8..15. */
    GERIUM_FORMAT_R64G64_SINT          = 76, /**< 128-bit signed integer format that has a 64-bit R component in bytes 0..7, and a 64-bit G component in bytes 8..15. */
    GERIUM_FORMAT_R64G64_SFLOAT        = 77, /**< 128-bit signed floating-point format that has a 64-bit R component in bytes 0..7, and a 64-bit G component in bytes 8..15. */
    GERIUM_FORMAT_R64G64B64_UINT       = 78, /**< 192-bit unsigned integer format that has a 64-bit R component in bytes 0..7, a 64-bit G component in bytes 8..15, and a 64-bit B component in bytes 16..23. */
    GERIUM_FORMAT_R64G64B64_SINT       = 79, /**< 192-bit signed integer format that has a 64-bit R component in bytes 0..7, a 64-bit G component in bytes 8..15, and a 64-bit B component in bytes 16..23. */
    GERIUM_FORMAT_R64G64B64_SFLOAT     = 80, /**< 192-bit signed floating-point format that has a 64-bit R component in bytes 0..7, a 64-bit G component in bytes 8..15, and a 64-bit B component in bytes 16..23. */
    GERIUM_FORMAT_R64G64B64A64_UINT    = 81, /**< 256-bit unsigned integer format that has a 64-bit R component in bytes 0..7, a 64-bit G component in bytes 8..15, a 64-bit B component in bytes 16..23, and a 64-bit A component in bytes 24..31. */
    GERIUM_FORMAT_R64G64B64A64_SINT    = 82, /**< 256-bit signed integer format that has a 64-bit R component in bytes 0..7, a 64-bit G component in bytes 8..15, a 64-bit B component in bytes 16..23, and a 64-bit A component in bytes 24..31. */
    GERIUM_FORMAT_R64G64B64A64_SFLOAT  = 83, /**< 256-bit signed floating-point format that has a 64-bit R component in bytes 0..7, a 64-bit G component in bytes 8..15, a 64-bit B component in bytes 16..23, and a 64-bit A component in bytes 24..31. */
    GERIUM_FORMAT_B4G4R4A4_UNORM       = 84, /**< 16-bit packed unsigned normalized format that has a 4-bit B component in bits 12..15, a 4-bit G component in bits 8..11, a 4-bit R component in bits 4..7, and a 4-bit A component in bits 0..3. */
    GERIUM_FORMAT_B5G5R5A1_UNORM       = 85, /**< 16-bit packed unsigned normalized format that has a 5-bit B component in bits 11..15, a 5-bit G component in bits 6..10, a 5-bit R component in bits 1..5, and a 1-bit A component in bit 0. */
    GERIUM_FORMAT_B5G6R5_UNORM         = 86, /**< 16-bit packed unsigned normalized format that has a 5-bit B component in bits 11..15, a 6-bit G component in bits 5..10, and a 5-bit R component in bits 0..4. */
    GERIUM_FORMAT_B8G8R8_UNORM         = 87, /**< 24-bit unsigned normalized format that has an 8-bit B component in byte 0, an 8-bit G component in byte 1, and an 8-bit R component in byte 2. */
    GERIUM_FORMAT_B8G8R8_SNORM         = 88, /**< 24-bit signed normalized format that has an 8-bit B component in byte 0, an 8-bit G component in byte 1, and an 8-bit R component in byte 2. */
    GERIUM_FORMAT_B8G8R8_UINT          = 89, /**< 24-bit unsigned integer format that has an 8-bit B component in byte 0, an 8-bit G component in byte 1, and an 8-bit R component in byte 2. */
    GERIUM_FORMAT_B8G8R8_SINT          = 90, /**< 24-bit signed integer format that has an 8-bit B component in byte 0, an 8-bit G component in byte 1, and an 8-bit R component in byte 2. */
    GERIUM_FORMAT_B8G8R8_SRGB          = 91, /**< 24-bit unsigned normalized format that has an 8-bit B component stored with sRGB nonlinear encoding in byte 0, an 8-bit G component stored with sRGB nonlinear encoding in byte 1, and an 8-bit R component stored with sRGB nonlinear encoding in byte 2. */
    GERIUM_FORMAT_B8G8R8_USCALED       = 92, /**< 24-bit unsigned scaled format that has an 8-bit B component in byte 0, an 8-bit G component in byte 1, and an 8-bit R component in byte 2. */
    GERIUM_FORMAT_B8G8R8_SSCALED       = 93, /**< 24-bit signed scaled format that has an 8-bit B component in byte 0, an 8-bit G component in byte 1, and an 8-bit R component in byte 2. */
    GERIUM_FORMAT_B8G8R8A8_UNORM       = 94, /**< 32-bit unsigned normalized format that has an 8-bit B component in byte 0, an 8-bit G component in byte 1, an 8-bit R component in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_B8G8R8A8_SNORM       = 95, /**< 32-bit signed normalized format that has an 8-bit B component in byte 0, an 8-bit G component in byte 1, an 8-bit R component in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_B8G8R8A8_UINT        = 96, /**< 32-bit unsigned integer format that has an 8-bit B component in byte 0, an 8-bit G component in byte 1, an 8-bit R component in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_B8G8R8A8_SINT        = 97, /**< 32-bit signed integer format that has an 8-bit B component in byte 0, an 8-bit G component in byte 1, an 8-bit R component in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_B8G8R8A8_SRGB        = 98, /**< 32-bit unsigned normalized format that has an 8-bit B component stored with sRGB nonlinear encoding in byte 0, an 8-bit G component stored with sRGB nonlinear encoding in byte 1, an 8-bit R component stored with sRGB nonlinear encoding in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_B8G8R8A8_USCALED     = 99, /**< 32-bit unsigned scaled format that has an 8-bit B component in byte 0, an 8-bit G component in byte 1, an 8-bit R component in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_B8G8R8A8_SSCALED     = 100, /**< 32-bit signed scaled format that has an 8-bit B component in byte 0, an 8-bit G component in byte 1, an 8-bit R component in byte 2, and an 8-bit A component in byte 3. */
    GERIUM_FORMAT_B10G11R11_UFLOAT     = 101, /**< 32-bit packed unsigned floating-point format that has a 10-bit B component in bits 22..31, an 11-bit G component in bits 11..21, an 11-bit R component in bits 0..10. */
    GERIUM_FORMAT_A1B5G5R5_UNORM       = 102, /**< 16-bit packed unsigned normalized format that has a 1-bit A component in bit 15, a 5-bit B component in bits 10..14, a 5-bit G component in bits 5..9, and a 5-bit R component in bits 0..4. */
    GERIUM_FORMAT_A1R5G5B5_UNORM       = 103, /**< 16-bit packed unsigned normalized format that has a 1-bit A component in bit 15, a 5-bit R component in bits 10..14, a 5-bit G component in bits 5..9, and a 5-bit B component in bits 0..4. */
    GERIUM_FORMAT_A2B10G10R10_UNORM    = 104, /**< 32-bit packed unsigned normalized format that has a 2-bit A component in bits 30..31, a 10-bit B component in bits 20..29, a 10-bit G component in bits 10..19, and a 10-bit R component in bits 0..9. */
    GERIUM_FORMAT_A2B10G10R10_SNORM    = 105, /**< 32-bit packed signed normalized format that has a 2-bit A component in bits 30..31, a 10-bit B component in bits 20..29, a 10-bit G component in bits 10..19, and a 10-bit R component in bits 0..9. */
    GERIUM_FORMAT_A2B10G10R10_UINT     = 106, /**< 32-bit packed unsigned integer format that has a 2-bit A component in bits 30..31, a 10-bit B component in bits 20..29, a 10-bit G component in bits 10..19, and a 10-bit R component in bits 0..9. */
    GERIUM_FORMAT_A2B10G10R10_SINT     = 107, /**< 32-bit packed signed integer format that has a 2-bit A component in bits 30..31, a 10-bit B component in bits 20..29, a 10-bit G component in bits 10..19, and a 10-bit R component in bits 0..9. */
    GERIUM_FORMAT_A2B10G10R10_USCALED  = 108, /**< 32-bit packed unsigned scaled integer format that has a 2-bit A component in bits 30..31, a 10-bit B component in bits 20..29, a 10-bit G component in bits 10..19, and a 10-bit R component in bits 0..9. */
    GERIUM_FORMAT_A2B10G10R10_SSCALED  = 109, /**< 32-bit packed signed scaled integer format that has a 2-bit A component in bits 30..31, a 10-bit B component in bits 20..29, a 10-bit G component in bits 10..19, and a 10-bit R component in bits 0..9. */
    GERIUM_FORMAT_A2R10G10B10_UNORM    = 110, /**< 32-bit packed unsigned normalized format that has a 2-bit A component in bits 30..31, a 10-bit R component in bits 20..29, a 10-bit G component in bits 10..19, and a 10-bit B component in bits 0..9. */
    GERIUM_FORMAT_A2R10G10B10_SNORM    = 111, /**< 32-bit packed signed normalized format that has a 2-bit A component in bits 30..31, a 10-bit R component in bits 20..29, a 10-bit G component in bits 10..19, and a 10-bit B component in bits 0..9. */
    GERIUM_FORMAT_A2R10G10B10_UINT     = 112, /**< 32-bit packed unsigned integer format that has a 2-bit A component in bits 30..31, a 10-bit R component in bits 20..29, a 10-bit G component in bits 10..19, and a 10-bit B component in bits 0..9. */
    GERIUM_FORMAT_A2R10G10B10_SINT     = 113, /**< 32-bit packed signed integer format that has a 2-bit A component in bits 30..31, a 10-bit R component in bits 20..29, a 10-bit G component in bits 10..19, and a 10-bit B component in bits 0..9. */
    GERIUM_FORMAT_A2R10G10B10_USCALED  = 114, /**< 32-bit packed unsigned scaled integer format that has a 2-bit A component in bits 30..31, a 10-bit R component in bits 20..29, a 10-bit G component in bits 10..19, and a 10-bit B component in bits 0..9. */
    GERIUM_FORMAT_A2R10G10B10_SSCALED  = 115, /**< 32-bit packed signed scaled integer format that has a 2-bit A component in bits 30..31, a 10-bit R component in bits 20..29, a 10-bit G component in bits 10..19, and a 10-bit B component in bits 0..9. */
    GERIUM_FORMAT_A4B4G4R4_UNORM       = 116, /**< 16-bit packed unsigned normalized format that has a 4-bit A component in bits 12..15, a 4-bit B component in bits 8..11, a 4-bit G component in bits 4..7, and a 4-bit R component in bits 0..3. */
    GERIUM_FORMAT_A4R4G4B4_UNORM       = 117, /**< 16-bit packed unsigned normalized format that has a 4-bit A component in bits 12..15, a 4-bit R component in bits 8..11, a 4-bit G component in bits 4..7, and a 4-bit B component in bits 0..3. */
    GERIUM_FORMAT_A8_UNORM             = 118, /**< 8-bit unsigned normalized format that has a single 8-bit A component. */
    GERIUM_FORMAT_A8B8G8R8_UNORM       = 119, /**< 32-bit packed unsigned normalized format that has an 8-bit A component in bits 24..31, an 8-bit B component in bits 16..23, an 8-bit G component in bits 8..15, and an 8-bit R component in bits 0..7. */
    GERIUM_FORMAT_A8B8G8R8_SNORM       = 120, /**< 32-bit packed signed normalized format that has an 8-bit A component in bits 24..31, an 8-bit B component in bits 16..23, an 8-bit G component in bits 8..15, and an 8-bit R component in bits 0..7. */
    GERIUM_FORMAT_A8B8G8R8_UINT        = 121, /**< 32-bit packed unsigned integer format that has an 8-bit A component in bits 24..31, an 8-bit B component in bits 16..23, an 8-bit G component in bits 8..15, and an 8-bit R component in bits 0..7. */
    GERIUM_FORMAT_A8B8G8R8_SINT        = 122, /**< 32-bit packed signed integer format that has an 8-bit A component in bits 24..31, an 8-bit B component in bits 16..23, an 8-bit G component in bits 8..15, and an 8-bit R component in bits 0..7. */
    GERIUM_FORMAT_A8B8G8R8_SRGB        = 123, /**< 32-bit packed unsigned normalized format that has an 8-bit A component in bits 24..31, an 8-bit B component stored with sRGB nonlinear encoding in bits 16..23, an 8-bit G component stored with sRGB nonlinear encoding in bits 8..15, and an 8-bit R component stored with sRGB nonlinear encoding in bits 0..7. */
    GERIUM_FORMAT_A8B8G8R8_USCALED     = 124, /**< 32-bit packed unsigned scaled integer format that has an 8-bit A component in bits 24..31, an 8-bit B component in bits 16..23, an 8-bit G component in bits 8..15, and an 8-bit R component in bits 0..7. */
    GERIUM_FORMAT_A8B8G8R8_SSCALED     = 125, /**< 32-bit packed signed scaled integer format that has an 8-bit A component in bits 24..31, an 8-bit B component in bits 16..23, an 8-bit G component in bits 8..15, and an 8-bit R component in bits 0..7. */
    GERIUM_FORMAT_S8_UINT              = 126, /**< 8-bit unsigned integer format that has 8 bits in the stencil component. */
    GERIUM_FORMAT_X8_D24_UNORM         = 127, /**< 32-bit format that has 24 unsigned normalized bits in the depth component and, optionally, 8 bits that are unused. */
    GERIUM_FORMAT_D16_UNORM            = 128, /**< 16-bit unsigned normalized format that has a single 16-bit depth component. */
    GERIUM_FORMAT_D16_UNORM_S8_UINT    = 129, /**< 24-bit format that has 16 unsigned normalized bits in the depth component and 8 unsigned integer bits in the stencil component. */
    GERIUM_FORMAT_D24_UNORM_S8_UINT    = 130, /**< 32-bit packed format that has 8 unsigned integer bits in the stencil component, and 24 unsigned normalized bits in the depth component. */
    GERIUM_FORMAT_D32_SFLOAT           = 131, /**< 32-bit signed floating-point format that has 32 bits in the depth component. */
    GERIUM_FORMAT_D32_SFLOAT_S8_UINT   = 132, /**< 32 signed float bits in the depth component and 8 unsigned integer bits in the stencil component. There are optionally 24 bits that are unused. */
    GERIUM_FORMAT_ASTC_4X4_UNORM       = 133, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_4X4_SRGB        = 134, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_4X4_SFLOAT      = 135, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_5X4_UNORM       = 136, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 5×4 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_5X4_SRGB        = 137, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 5×4 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_5X4_SFLOAT      = 138, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 5×4 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_5X5_UNORM       = 139, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 5×5 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_5X5_SRGB        = 140, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 5×5 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_5X5_SFLOAT      = 141, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 5×5 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_6X5_UNORM       = 142, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 6×5 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_6X5_SRGB        = 143, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 6×5 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_6X5_SFLOAT      = 144, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 6×5 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_6X6_UNORM       = 145, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 6×6 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_6X6_SRGB        = 146, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 6×6 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_6X6_SFLOAT      = 147, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 6×6 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_8X5_UNORM       = 148, /**< ASTC compressed format where each 128-bit compressed texel block encodes an 8×5 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_8X5_SRGB        = 149, /**< ASTC compressed format where each 128-bit compressed texel block encodes an 8×5 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_8X5_SFLOAT      = 150, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 8×5 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_8X6_UNORM       = 151, /**< ASTC compressed format where each 128-bit compressed texel block encodes an 8×6 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_8X6_SRGB        = 152, /**< ASTC compressed format where each 128-bit compressed texel block encodes an 8×6 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_8X6_SFLOAT      = 153, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 8×6 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_8X8_UNORM       = 154, /**< ASTC compressed format where each 128-bit compressed texel block encodes an 8×8 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_8X8_SRGB        = 155, /**< ASTC compressed format where each 128-bit compressed texel block encodes an 8×8 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_8X8_SFLOAT      = 156, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 8×8 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_10X5_UNORM      = 157, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 10×5 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_10X5_SRGB       = 158, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 10×5 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_10X5_SFLOAT     = 159, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 10×5 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_10X6_UNORM      = 160, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 10×6 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_10X6_SRGB       = 161, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 10×6 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_10X6_SFLOAT     = 162, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 10×6 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_10X8_UNORM      = 163, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 10×8 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_10X8_SRGB       = 164, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 10×8 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_10X8_SFLOAT     = 165, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 10×8 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_10X10_UNORM     = 166, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 10×10 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_10X10_SRGB      = 167, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 10×10 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_10X10_SFLOAT    = 168, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 10×10 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_12X10_UNORM     = 169, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 12×10 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_12X10_SRGB      = 170, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 12×10 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_12X10_SFLOAT    = 171, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 12×10 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_ASTC_12X12_UNORM     = 172, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 12×12 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_ASTC_12X12_SRGB      = 173, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 12×12 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ASTC_12X12_SFLOAT    = 174, /**< ASTC compressed format where each 128-bit compressed texel block encodes a 12×12 rectangle of signed floating-point RGBA texel data. */
    GERIUM_FORMAT_BC1_RGB_UNORM        = 175, /**< block-compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGB texel data. This format has no alpha and is considered opaque. */
    GERIUM_FORMAT_BC1_RGB_SRGB         = 176, /**< block-compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGB texel data with sRGB nonlinear encoding. This format has no alpha and is considered opaque. */
    GERIUM_FORMAT_BC1_RGBA_UNORM       = 177, /**< block-compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGB texel data, and provides 1 bit of alpha. */
    GERIUM_FORMAT_BC1_RGBA_SRGB        = 178, /**< block-compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGB texel data with sRGB nonlinear encoding, and provides 1 bit of alpha. */
    GERIUM_FORMAT_BC2_UNORM            = 179, /**< block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data with the first 64 bits encoding alpha values followed by 64 bits encoding RGB values. */
    GERIUM_FORMAT_BC2_SRGB             = 180, /**< block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data with the first 64 bits encoding alpha values followed by 64 bits encoding RGB values with sRGB nonlinear encoding. */
    GERIUM_FORMAT_BC3_UNORM            = 181, /**< block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data with the first 64 bits encoding alpha values followed by 64 bits encoding RGB values. */
    GERIUM_FORMAT_BC3_SRGB             = 182, /**< block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data with the first 64 bits encoding alpha values followed by 64 bits encoding RGB values with sRGB nonlinear encoding. */
    GERIUM_FORMAT_BC4_UNORM            = 183, /**< block-compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized red texel data. */
    GERIUM_FORMAT_BC4_SNORM            = 184, /**< block-compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of signed normalized red texel data. */
    GERIUM_FORMAT_BC5_UNORM            = 185, /**< block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RG texel data with the first 64 bits encoding red values followed by 64 bits encoding green values. */
    GERIUM_FORMAT_BC5_SNORM            = 186, /**< block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of signed normalized RG texel data with the first 64 bits encoding red values followed by 64 bits encoding green values. */
    GERIUM_FORMAT_BC6H_UFLOAT          = 187, /**< block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned floating-point RGB texel data. */
    GERIUM_FORMAT_BC6H_SFLOAT          = 188, /**< block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of signed floating-point RGB texel data. */
    GERIUM_FORMAT_BC7_UNORM            = 189, /**< block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_BC7_SRGB             = 190, /**< block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_ETC2_R8G8B8_UNORM    = 191, /**< ETC2 compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGB texel data. This format has no alpha and is considered opaque. */
    GERIUM_FORMAT_ETC2_R8G8B8_SRGB     = 192, /**< ETC2 compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGB texel data with sRGB nonlinear encoding. This format has no alpha and is considered opaque. */
    GERIUM_FORMAT_ETC2_R8G8B8A1_UNORM  = 193, /**< ETC2 compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGB texel data, and provides 1 bit of alpha. */
    GERIUM_FORMAT_ETC2_R8G8B8A1_SRGB   = 194, /**< ETC2 compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGB texel data with sRGB nonlinear encoding, and provides 1 bit of alpha. */
    GERIUM_FORMAT_ETC2_R8G8B8A8_UNORM  = 195, /**< ETC2 compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data with the first 64 bits encoding alpha values followed by 64 bits encoding RGB values. */
    GERIUM_FORMAT_ETC2_R8G8B8A8_SRGB   = 196, /**< ETC2 compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data with the first 64 bits encoding alpha values followed by 64 bits encoding RGB values with sRGB nonlinear encoding applied. */
    GERIUM_FORMAT_EAC_R11_UNORM        = 197, /**< EAC compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized red texel data. */
    GERIUM_FORMAT_EAC_R11_SNORM        = 198, /**< EAC compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of signed normalized red texel data. */
    GERIUM_FORMAT_EAC_R11G11_UNORM     = 199, /**< EAC compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RG texel data with the first 64 bits encoding red values followed by 64 bits encoding green values. */
    GERIUM_FORMAT_EAC_R11G11_SNORM     = 200, /**< EAC compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of signed normalized RG texel data with the first 64 bits encoding red values followed by 64 bits encoding green values. */
    GERIUM_FORMAT_PVRTC1_2BPP_UNORM    = 201, /**< PVRTC compressed format where each 64-bit compressed texel block encodes an 8×4 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_PVRTC1_2BPP_SRGB     = 202, /**< PVRTC compressed format where each 64-bit compressed texel block encodes an 8×4 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_PVRTC1_4BPP_UNORM    = 203, /**< PVRTC compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_PVRTC1_4BPP_SRGB     = 204, /**< PVRTC compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_PVRTC2_2BPP_UNORM    = 205, /**< PVRTC compressed format where each 64-bit compressed texel block encodes an 8×4 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_PVRTC2_2BPP_SRGB     = 206, /**< PVRTC compressed format where each 64-bit compressed texel block encodes an 8×4 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_PVRTC2_4BPP_UNORM    = 207, /**< PVRTC compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data. */
    GERIUM_FORMAT_PVRTC2_4BPP_SRGB     = 208, /**< PVRTC compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. */
    GERIUM_FORMAT_MAX_ENUM             = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_format_t;

/**
 * @brief   Polygon rendering modes.
 * @details Controls how polygons are rasterized.
 */
typedef enum
{
    GERIUM_POLYGON_MODE_FILL     = 0, /**< Renders filled polygon (standard solid rendering mode). */
    GERIUM_POLYGON_MODE_LINE     = 1, /**< Renders polygon edges as lines (wireframe mode). */
    GERIUM_POLYGON_MODE_POINT    = 2, /**< Renders only polygon vertices as points. */
    GERIUM_POLYGON_MODE_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_polygon_mode_t;

/**
 * @brief   Primitive topologies.
 * @details Defines how vertices are assembled into primitives.
 */
typedef enum
{
    GERIUM_PRIMITIVE_TOPOLOGY_POINT_LIST     = 0, /**< Treats each vertex as individual point primitive. */
    GERIUM_PRIMITIVE_TOPOLOGY_LINE_LIST      = 1, /**< Connects every 2 vertices as separate line segments. */
    GERIUM_PRIMITIVE_TOPOLOGY_LINE_STRIP     = 2, /**< Connects vertices as continuous line strip. */
    GERIUM_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST  = 3, /**< Groups every 3 vertices as separate triangles. */
    GERIUM_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4, /**< Creates triangle strip from vertex sequence. */
    GERIUM_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN   = 5, /**< Forms triangle fan from shared first vertex. */
    GERIUM_PRIMITIVE_TOPOLOGY_MAX_ENUM       = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_primitive_topology_t;

/**
 * @brief   Face culling modes.
 * @details Controls which polygon faces are discarded.
 */
typedef enum
{
    GERIUM_CULL_MODE_NONE           = 0, /**< Disables face culling (renders all polygons). */
    GERIUM_CULL_MODE_FRONT          = 1, /**< Culls front-facing polygons. */
    GERIUM_CULL_MODE_BACK           = 2, /**< Culls back-facing polygons. */
    GERIUM_CULL_MODE_FRONT_AND_BACK = 3, /**< Culls all faces. */
    GERIUM_CULL_MODE_MAX_ENUM       = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_cull_mode_t;

/**
 * @brief   Front face orientation.
 * @details Defines what constitutes a front-facing polygon.
 */
typedef enum
{
    GERIUM_FRONT_FACE_COUNTER_CLOCKWISE = 0, /**< Front face has counter-clockwise winding. */
    GERIUM_FRONT_FACE_CLOCKWISE         = 1, /**< Front face has clockwise winding. */
    GERIUM_FRONT_FACE_MAX_ENUM          = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_front_face_t;

/**
 * @brief   Comparison operator for depth, stencil, and sampler operations.
 * @details Comparison operators compare a *reference* and a *test* value, and return a true or false value depending on the comparison operator chosen.
 */
typedef enum
{
    GERIUM_COMPARE_OP_NEVER            = 0, /**< Comparison always evaluates false. */
    GERIUM_COMPARE_OP_ALWAYS           = 1, /**< Comparison always evaluates true. */
    GERIUM_COMPARE_OP_LESS             = 2, /**< Comparison evaluates whether *reference* is less than *test*. */
    GERIUM_COMPARE_OP_LESS_OR_EQUAL    = 3, /**< Comparison evaluates whether *reference* is less than or equal to *test*. */
    GERIUM_COMPARE_OP_GREATER          = 4, /**< Comparison evaluates whether *reference* is greater than *test*. */
    GERIUM_COMPARE_OP_GREATER_OR_EQUAL = 5, /**< Comparison evaluates whether *reference* is greater than or equal to *test*. */
    GERIUM_COMPARE_OP_EQUAL            = 6, /**< Comparison evaluates *reference* and *test* for equality. */
    GERIUM_COMPARE_OP_NOT_EQUAL        = 7, /**< Comparison evaluates that *reference* is not equal to *test*. */
    GERIUM_COMPARE_OP_MAX_ENUM         = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_compare_op_t;

/**
 * @brief   Stencil comparison function.
 * @details Possible values of the gerium_stencil_op_state_t::fail_op, gerium_stencil_op_state_t::pass_op and
 *          gerium_stencil_op_state_t::depth_fail_op, specifying what happens to the stored stencil
 *          value if this or certain subsequent tests fail or pass.
 * @sa      gerium_stencil_op_state_t
 */
typedef enum
{
    GERIUM_STENCIL_OP_KEEP                = 0, /**< Keeps the current value. */
    GERIUM_STENCIL_OP_ZERO                = 1, /**< Sets the value to 0. */
    GERIUM_STENCIL_OP_REPLACE             = 2, /**< Sets the value to *reference*. */
    GERIUM_STENCIL_OP_INVERT              = 3, /**< Bitwise-inverts the current value. */
    GERIUM_STENCIL_OP_INCREMENT_AND_CLAMP = 4, /**< Increments the current value and clamps to the maximum representable unsigned value. */
    GERIUM_STENCIL_OP_DECREMENT_AND_CLAMP = 5, /**< Decrements the current value and clamps to 0. */
    GERIUM_STENCIL_OP_INCREMENT_AND_WRAP  = 6, /**< Increments the current value and wraps to 0 when the maximum value would have been exceeded. */
    GERIUM_STENCIL_OP_DECREMENT_AND_WRAP  = 7, /**< Decrements the current value and wraps to the maximum possible value when the value would go below 0. */
    GERIUM_STENCIL_OP_MAX_ENUM            = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_stencil_op_t;

/**
 * @brief   Logical operations.
 * @details Pixel-wise logical operations between source and destination.
 * @note    Logical operations are controlled by the gerium_color_blend_state_t::logic_op_enable and gerium_color_blend_state_t::logic_op.
 * @sa      gerium_color_blend_state_t
 */
typedef enum
{
    GERIUM_LOGIC_OP_CLEAR         = 0, /**< Clears destination to 0 (`0`). */
    GERIUM_LOGIC_OP_SET           = 1, /**< Sets destination to 1 (`1`). */
    GERIUM_LOGIC_OP_NO_OP         = 2, /**< Leaves destination unchanged (`dst`). */
    GERIUM_LOGIC_OP_COPY          = 3, /**< Copies source to destination (`src`). */
    GERIUM_LOGIC_OP_COPY_INVERTED = 4, /**< Copies inverted source to destination (`~src`). */
    GERIUM_LOGIC_OP_AND           = 5, /**< Performs bitwise AND (`src & dst`). */
    GERIUM_LOGIC_OP_AND_REVERSE   = 6, /**< Performs bitwise AND with inverted dst (`src & ~dst`). */
    GERIUM_LOGIC_OP_AND_INVERTED  = 7, /**< Performs bitwise AND with inverted src (`~src & dst`). */
    GERIUM_LOGIC_OP_NAND          = 8, /**< Performs bitwise NAND (`~(src & dst)`). */
    GERIUM_LOGIC_OP_OR            = 9, /**< Performs bitwise OR (`src | dst`). */
    GERIUM_LOGIC_OP_OR_REVERSE    = 10, /**< Performs bitwise OR with inverted dst (`src | ~dst`). */
    GERIUM_LOGIC_OP_OR_INVERTED   = 11, /**< Performs bitwise OR with inverted src (`~src | dst`). */
    GERIUM_LOGIC_OP_NOR           = 12, /**< Performs bitwise NOR (`~(src | dst)`). */
    GERIUM_LOGIC_OP_XOR           = 13, /**< Performs bitwise XOR (`src ^ dst`). */
    GERIUM_LOGIC_OP_EQUIVALENT    = 14, /**< Performs bitwise equality (`~(src ^ dst)`). */
    GERIUM_LOGIC_OP_INVERT        = 15, /**< Inverts destination (`~dst`). */
    GERIUM_LOGIC_OP_MAX_ENUM      = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_logic_op_t;

/**
 * @brief   Blending operations.
 * @details Once the source and destination blend factors have been selected, they along with
 *          the source and destination components are passed to the blending operations. RGB
 *          and alpha components can use different operations.
 */
typedef enum
{
    GERIUM_BLEND_OP_ADD              = 0, /**< Adds source and destination (`src + dst`). */
    GERIUM_BLEND_OP_SUBTRACT         = 1, /**< Subtracts destination from source (`src - dst`). */
    GERIUM_BLEND_OP_REVERSE_SUBTRACT = 2, /**< Subtracts source from destination (`dst - src`). */
    GERIUM_BLEND_OP_MIN              = 3, /**< Takes component-wise minimum (`min(src, dst)`). */
    GERIUM_BLEND_OP_MAX              = 4, /**< Takes component-wise maximum (`max(src, dst)`). */
    GERIUM_BLEND_OP_MAX_ENUM         = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_blend_op_t;

/**
 * @brief   Render pass operations.
 * @details Initial load/store operations for render passes.
 * @sa      gerium_resource_output_t
 */
typedef enum
{
    GERIUM_RENDER_PASS_OP_DONT_CARE = 0, /**< Contents may be discarded. */
    GERIUM_RENDER_PASS_OP_CLEAR     = 1, /**< Clears attachment to specified value (gerium_resource_output_t::clear_color_attachment or gerium_resource_output_t::clear_depth_stencil_attachment). */
    GERIUM_RENDER_PASS_OP_MAX_ENUM  = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_render_pass_op_t;

/**
 * @brief   Blending factors.
 * @details Multiplicative factors for blending operations.
 */
typedef enum
{
    GERIUM_BLEND_FACTOR_ZERO                     = 0, /**< Multiplies by `RGB(0,0,0)` and `A(0)`. */
    GERIUM_BLEND_FACTOR_ONE                      = 1, /**< Multiplies by `RGB(1,1,1)` and `A(1)`. */
    GERIUM_BLEND_FACTOR_SRC_COLOR                = 2, /**< Multiplies by source color `RGB(src0.rgb)` and `A(src0.a)`. */
    GERIUM_BLEND_FACTOR_SRC_ALPHA                = 3, /**< Multiplies by source alpha `RGB(src0.aaa)` and `A(src0.a)`. */
    GERIUM_BLEND_FACTOR_SRC_ALPHA_SATURATE       = 4, /**< Multiplies by `RGB(min(src0.a,1-dst.a))` and `A(1)`. */
    GERIUM_BLEND_FACTOR_DST_COLOR                = 5, /**< Multiplies by destination color `RGB(dst.rgb)` and `A(dst.a)`. */
    GERIUM_BLEND_FACTOR_DST_ALPHA                = 6, /**< Multiplies by destination alpha `RGB(dst.aaa)` and `A(dst.a)`. */
    GERIUM_BLEND_FACTOR_CONSTANT_COLOR           = 7, /**< Multiplies by blend constant color `RGB(cnst.rgb)` and `A(cnst.a)`. */
    GERIUM_BLEND_FACTOR_CONSTANT_ALPHA           = 8, /**< Multiplies by blend constant alpha `RGB(cnst.aaa)` and `A(cnst.a)`. */
    GERIUM_BLEND_FACTOR_SRC1_COLOR               = 9, /**< Multiplies by source1 color `RGB(src1.rgb)` and `A(src1.a)` (dual-source blending). */
    GERIUM_BLEND_FACTOR_SRC1_ALPHA               = 10, /**< Multiplies by source1 alpha `RGB(src1.aaa)` and `A(src1.a)` (dual-source blending). */
    GERIUM_BLEND_FACTOR_ONE_MINUS_SRC_COLOR      = 11, /**< Multiplies by `RGB(1-src0.rgb)` and `A(1-src0.a)`. */
    GERIUM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA      = 12, /**< Multiplies by `RGB(1-src0.aaa)` and `A(1-src0.a)`. */
    GERIUM_BLEND_FACTOR_ONE_MINUS_DST_COLOR      = 13, /**< Multiplies by `RGB(1-dst.rgb)` and `A(1-dst.a)`. */
    GERIUM_BLEND_FACTOR_ONE_MINUS_DST_ALPHA      = 14, /**< Multiplies by `RGB(1-dst.aaa)` and `A(1-dst.a)`. */
    GERIUM_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = 15, /**< Multiplies by `RGB(1-cnst.rgb)` and `A(1-cnst.a)`. */
    GERIUM_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = 16, /**< Multiplies by `RGB(1-cnst.aaa)` and `A(1-cnst.a)`. */
    GERIUM_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR     = 17, /**< Multiplies by `RGB(1-src1.rgb)` and `A(1-src1.a)`. */
    GERIUM_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA     = 18, /**< Multiplies by `RGB(1-src1.aaa)` and `A(1-src1.a)`. */
    GERIUM_BLEND_FACTOR_MAX_ENUM                 = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_blend_factor_t;

/**
 * @brief   Color components.
 * @details Bitmask controlling which components are written to the framebuffer.
 */
typedef enum
{
    GERIUM_COLOR_COMPONENT_R_BIT    = 1, /**< Red channel. */
    GERIUM_COLOR_COMPONENT_G_BIT    = 2, /**< Green channel. */
    GERIUM_COLOR_COMPONENT_B_BIT    = 4, /**< Blue channel. */
    GERIUM_COLOR_COMPONENT_A_BIT    = 8, /**< Alpha channel. */
    GERIUM_COLOR_COMPONENT_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_color_component_flags_t;
GERIUM_FLAGS(gerium_color_component_flags_t)

/**
 * @brief   Buffer usage flags.
 * @details Bitmask specifying allowed usage of a buffer.
 */
typedef enum
{
    GERIUM_BUFFER_USAGE_NONE_BIT     = 0, /**< No specific usage. */
    GERIUM_BUFFER_USAGE_VERTEX_BIT   = 1, /**< The buffer is suitable for passing as an vertex buffer to ::gerium_command_buffer_bind_vertex_buffer. */
    GERIUM_BUFFER_USAGE_INDEX_BIT    = 2, /**< The buffer is suitable for passing as an index buffer to ::gerium_command_buffer_bind_index_buffer. */
    GERIUM_BUFFER_USAGE_UNIFORM_BIT  = 4, /**< The buffer is suitable for use as a uniform buffer (UBO). */
    GERIUM_BUFFER_USAGE_STORAGE_BIT  = 8, /**< The buffer is suitable for use as a storage buffer (SSBO). */
    GERIUM_BUFFER_USAGE_INDIRECT_BIT = 16, /**< Used for indirect drawing parameters. The buffer is suitable for passing as the buffer parameter to ::gerium_command_buffer_draw_indirect, ::gerium_command_buffer_draw_indexed_indirect and ::gerium_command_buffer_draw_mesh_tasks_indirect. */
    GERIUM_BUFFER_USAGE_MAX_ENUM     = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_buffer_usage_flags_t;
GERIUM_FLAGS(gerium_buffer_usage_flags_t)

/**
 * @brief   Texture dimensionality.
 * @details Specifies the type of texture.
 */
typedef enum
{
    GERIUM_TEXTURE_TYPE_1D       = 0, /**< 1-dimensional texture. */
    GERIUM_TEXTURE_TYPE_2D       = 1, /**< 2-dimensional texture. */
    GERIUM_TEXTURE_TYPE_3D       = 2, /**< 3-dimensional texture. */
    GERIUM_TEXTURE_TYPE_CUBE     = 3, /**< Cube map texture (6 square faces). */
    GERIUM_TEXTURE_TYPE_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_texture_type_t;

/**
 * @brief   Texture filtering modes.
 * @details Texel sampling methods.
 */
typedef enum
{
    GERIUM_FILTER_NEAREST  = 0, /**< Point sampling (returns nearest texel without interpolation). */
    GERIUM_FILTER_LINEAR   = 1, /**< Bilinear/trilinear interpolation. */
    GERIUM_FILTER_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_filter_t;

/**
 * @brief   Texture addressing modes.
 * @details Behavior when sampling beyond texture coordinates.
 */
typedef enum
{
    GERIUM_ADDRESS_MODE_REPEAT               = 0, /**< Tile the texture infinitely (wrap-around). */
    GERIUM_ADDRESS_MODE_MIRRORED_REPEAT      = 1, /**< Tile with mirroring at boundaries. */
    GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE        = 2, /**< Extend edge texels beyond [0,1] range. */
    GERIUM_ADDRESS_MODE_CLAMP_TO_BORDER      = 3, /**< Use specified border color beyond [0,1]. */
    GERIUM_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE = 4, /**< Mirror once then clamp to edge. */
    GERIUM_ADDRESS_MODE_MAX_ENUM             = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_address_mode_t;

/**
 * @brief   Sampler reduction modes.
 * @details Special texture filtering operations.
 */
typedef enum
{
    GERIUM_REDUCTION_MODE_WEIGHTED_AVERAGE = 0, /**< Texel values are combined by computing a weighted average of values in the footprint. */
    GERIUM_REDUCTION_MODE_MIN              = 1, /**< Texel values are combined by taking the component-wise minimum of values in the footprint with non-zero weights. */
    GERIUM_REDUCTION_MODE_MAX              = 2, /**< Texel values are combined by taking the component-wise maximum of values in the footprint with non-zero weights. */
    GERIUM_REDUCTION_MODE_MAX_ENUM         = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_reduction_mode_t;

/**
 * @brief   Specifies how the resource will be used.
 * @details This enumeration is necessary for setting barriers for the transition of resources between different stages of the pipeline and for the correct construction of the frame graph.
 * @sa      gerium_resource_input_t
 * @sa      gerium_resource_output_t
 */
typedef enum
{
    GERIUM_RESOURCE_TYPE_BUFFER     = 0, /**< Specifies that the resource will be used as a buffer. */
    GERIUM_RESOURCE_TYPE_TEXTURE    = 1, /**< Specifies that the resource will be used as a texture. */
    GERIUM_RESOURCE_TYPE_ATTACHMENT = 2, /**< Specifies that the resource will be used as a render target. */
    GERIUM_RESOURCE_TYPE_REFERENCE  = 3, /**< Used as reference input resources, which is necessary in some cases to clarify dependencies for correct graph computation during compilation. */
    GERIUM_RESOURCE_TYPE_MAX_ENUM   = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_resource_type_t;

/**
 * @brief   Vertex input rates.
 * @details Frequency of vertex attribute updates.
 */
typedef enum
{
    GERIUM_VERTEX_RATE_PER_VERTEX   = 0, /**< Fetch new data for each vertex. */
    GERIUM_VERTEX_RATE_PER_INSTANCE = 1, /**< Fetch new data for each instance. */
    GERIUM_VERTEX_RATE_MAX_ENUM     = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_vertex_rate_t;

/**
 * @brief   Index buffer formats.
 * @details Index data types for indexed rendering.
 */
typedef enum
{
    GERIUM_INDEX_TYPE_UINT16   = 0, /**< 16-bit unsigned indices. */
    GERIUM_INDEX_TYPE_UINT32   = 1, /**< 32-bit unsigned indices. */
    GERIUM_INDEX_TYPE_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_index_type_t;

/**
 * @brief   Shader stage types
 * @details Pipeline programmable stages
 */
typedef enum
{
    GERIUM_SHADER_TYPE_VERTEX   = 0, /**< Vertex processing stage */
    GERIUM_SHADER_TYPE_FRAGMENT = 1, /**< Pixel shading stage */
    GERIUM_SHADER_TYPE_GEOMETRY = 2, /**< Geometry processing stage */
    GERIUM_SHADER_TYPE_COMPUTE  = 3, /**< General compute operations */
    GERIUM_SHADER_TYPE_TASK     = 4, /**< Task shader (mesh shading pipeline) */
    GERIUM_SHADER_TYPE_MESH     = 5, /**< Mesh shader (mesh shading pipeline) */
    GERIUM_SHADER_TYPE_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_shader_type_t;

/**
 * @brief   Shader languages
 * @details Supported shading languages
 */
typedef enum
{
    GERIUM_SHADER_LANGUAGE_UNKNOWN  = 0, /**< Unknown language */
    GERIUM_SHADER_LANGUAGE_SPIRV    = 1, /**< Standard Portable Intermediate Representation */
    GERIUM_SHADER_LANGUAGE_GLSL     = 2, /**< OpenGL Shading Language */
    GERIUM_SHADER_LANGUAGE_HLSL     = 3, /**< High-Level Shading Language */
    GERIUM_SHADER_LANGUAGE_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} gerium_shader_language_t;

GERIUM_END

#endif /* GERIUM_ENUMS_H */
