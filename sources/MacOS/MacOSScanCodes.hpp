#ifndef GERIUM_MAC_OS_MAC_OS_SCAN_CODES_HPP
#define GERIUM_MAC_OS_MAC_OS_SCAN_CODES_HPP

#include "../Gerium.hpp"

#include <Carbon/Carbon.h>

#import <AppKit/NSEvent.h>

namespace gerium::macos {

gerium_inline std::pair<gerium_scancode_t, gerium_key_code_t> toScanCode(gerium_uint16_t keyCode, gerium_key_mod_flags_t mods) noexcept {
    bool shift = mods & GERIUM_KEY_MOD_SHIFT;
    bool numlock = mods & GERIUM_KEY_MOD_NUM_LOCK;
    
    switch (keyCode) {
        case kVK_ANSI_0: return { GERIUM_SCANCODE_0, shift ? GERIUM_KEY_CODE_PAREN_RIGHT : GERIUM_KEY_CODE_0 };
        case kVK_ANSI_1: return { GERIUM_SCANCODE_1, shift ? GERIUM_KEY_CODE_EXCLAIM : GERIUM_KEY_CODE_1 };
        case kVK_ANSI_2: return { GERIUM_SCANCODE_2, shift ? GERIUM_KEY_CODE_AT : GERIUM_KEY_CODE_2 };
        case kVK_ANSI_3: return { GERIUM_SCANCODE_3, shift ? GERIUM_KEY_CODE_HASH : GERIUM_KEY_CODE_3 };
        case kVK_ANSI_4: return { GERIUM_SCANCODE_4, shift ? GERIUM_KEY_CODE_DOLLAR : GERIUM_KEY_CODE_4 };
        case kVK_ANSI_5: return { GERIUM_SCANCODE_5, shift ? GERIUM_KEY_CODE_PERCENT : GERIUM_KEY_CODE_5 };
        case kVK_ANSI_6: return { GERIUM_SCANCODE_6, shift ? GERIUM_KEY_CODE_CARET : GERIUM_KEY_CODE_6 };
        case kVK_ANSI_7: return { GERIUM_SCANCODE_7, shift ? GERIUM_KEY_CODE_AMPERSAND : GERIUM_KEY_CODE_7 };
        case kVK_ANSI_8: return { GERIUM_SCANCODE_8, shift ? GERIUM_KEY_CODE_ASTERISK : GERIUM_KEY_CODE_8 };
        case kVK_ANSI_9: return { GERIUM_SCANCODE_9, shift ? GERIUM_KEY_CODE_PAREN_LEFT : GERIUM_KEY_CODE_9 };
    case kVK_ANSI_A: return { GERIUM_SCANCODE_A, GERIUM_KEY_CODE_A };
    case kVK_ANSI_B: return { GERIUM_SCANCODE_B, GERIUM_KEY_CODE_B };
    case kVK_ANSI_C: return { GERIUM_SCANCODE_C, GERIUM_KEY_CODE_C };
    case kVK_ANSI_D: return { GERIUM_SCANCODE_D, GERIUM_KEY_CODE_D };
    case kVK_ANSI_E: return { GERIUM_SCANCODE_E, GERIUM_KEY_CODE_E };
    case kVK_ANSI_F: return { GERIUM_SCANCODE_F, GERIUM_KEY_CODE_F };
    case kVK_ANSI_G: return { GERIUM_SCANCODE_G, GERIUM_KEY_CODE_G };
    case kVK_ANSI_H: return { GERIUM_SCANCODE_H, GERIUM_KEY_CODE_H };
    case kVK_ANSI_I: return { GERIUM_SCANCODE_I, GERIUM_KEY_CODE_I };
    case kVK_ANSI_J: return { GERIUM_SCANCODE_J, GERIUM_KEY_CODE_J };
    case kVK_ANSI_K: return { GERIUM_SCANCODE_K, GERIUM_KEY_CODE_K };
    case kVK_ANSI_L: return { GERIUM_SCANCODE_L, GERIUM_KEY_CODE_L };
    case kVK_ANSI_M: return { GERIUM_SCANCODE_M, GERIUM_KEY_CODE_M };
    case kVK_ANSI_N: return { GERIUM_SCANCODE_N, GERIUM_KEY_CODE_N };
    case kVK_ANSI_O: return { GERIUM_SCANCODE_O, GERIUM_KEY_CODE_O };
    case kVK_ANSI_P: return { GERIUM_SCANCODE_P, GERIUM_KEY_CODE_P };
    case kVK_ANSI_Q: return { GERIUM_SCANCODE_Q, GERIUM_KEY_CODE_Q };
    case kVK_ANSI_R: return { GERIUM_SCANCODE_R, GERIUM_KEY_CODE_R };
    case kVK_ANSI_S: return { GERIUM_SCANCODE_S, GERIUM_KEY_CODE_S };
    case kVK_ANSI_T: return { GERIUM_SCANCODE_T, GERIUM_KEY_CODE_T };
    case kVK_ANSI_U: return { GERIUM_SCANCODE_U, GERIUM_KEY_CODE_U };
    case kVK_ANSI_V: return { GERIUM_SCANCODE_V, GERIUM_KEY_CODE_V };
    case kVK_ANSI_W: return { GERIUM_SCANCODE_W, GERIUM_KEY_CODE_W };
    case kVK_ANSI_X: return { GERIUM_SCANCODE_X, GERIUM_KEY_CODE_X };
    case kVK_ANSI_Y: return { GERIUM_SCANCODE_Y, GERIUM_KEY_CODE_Y };
    case kVK_ANSI_Z: return { GERIUM_SCANCODE_Z, GERIUM_KEY_CODE_Z };
    case kVK_F1: return { GERIUM_SCANCODE_F1, GERIUM_KEY_CODE_F1 };
    case kVK_F2: return { GERIUM_SCANCODE_F2, GERIUM_KEY_CODE_F2 };
    case kVK_F3: return { GERIUM_SCANCODE_F3, GERIUM_KEY_CODE_F3 };
    case kVK_F4: return { GERIUM_SCANCODE_F4, GERIUM_KEY_CODE_F4 };
    case kVK_F5: return { GERIUM_SCANCODE_F5, GERIUM_KEY_CODE_F5 };
    case kVK_F6: return { GERIUM_SCANCODE_F6, GERIUM_KEY_CODE_F6 };
    case kVK_F7: return { GERIUM_SCANCODE_F7, GERIUM_KEY_CODE_F7 };
    case kVK_F8: return { GERIUM_SCANCODE_F8, GERIUM_KEY_CODE_F8 };
    case kVK_F9: return { GERIUM_SCANCODE_F9, GERIUM_KEY_CODE_F9 };
    case kVK_F10: return { GERIUM_SCANCODE_F10, GERIUM_KEY_CODE_F10 };
    case kVK_F11: return { GERIUM_SCANCODE_F11, GERIUM_KEY_CODE_F11 };
    case kVK_F12: return { GERIUM_SCANCODE_F12, GERIUM_KEY_CODE_F12 };
    case kVK_F13: return { GERIUM_SCANCODE_F13, GERIUM_KEY_CODE_F13 };
    case kVK_F14: return { GERIUM_SCANCODE_F14, GERIUM_KEY_CODE_F14 };
    case kVK_F15: return { GERIUM_SCANCODE_F15, GERIUM_KEY_CODE_F15 };
    case kVK_F16: return { GERIUM_SCANCODE_F16, GERIUM_KEY_CODE_F16 };
    case kVK_F17: return { GERIUM_SCANCODE_F17, GERIUM_KEY_CODE_F17 };
    case kVK_F18: return { GERIUM_SCANCODE_F18, GERIUM_KEY_CODE_F18 };
    case kVK_F19: return { GERIUM_SCANCODE_F19, GERIUM_KEY_CODE_F19 };
    case kVK_F20: return { GERIUM_SCANCODE_F20, GERIUM_KEY_CODE_F20 };
        case kVK_ANSI_Keypad0: return { GERIUM_SCANCODE_NUMPAD_0, numlock ? GERIUM_KEY_CODE_0 : GERIUM_KEY_CODE_INSERT};
        case kVK_ANSI_Keypad1: return { GERIUM_SCANCODE_NUMPAD_1, numlock ? GERIUM_KEY_CODE_1 : GERIUM_KEY_CODE_END};
        case kVK_ANSI_Keypad2: return { GERIUM_SCANCODE_NUMPAD_2, numlock ? GERIUM_KEY_CODE_2 : GERIUM_KEY_CODE_ARROW_DOWN };
        case kVK_ANSI_Keypad3: return { GERIUM_SCANCODE_NUMPAD_3, numlock ? GERIUM_KEY_CODE_3 : GERIUM_KEY_CODE_PAGE_DOWN };
        case kVK_ANSI_Keypad4: return { GERIUM_SCANCODE_NUMPAD_4, numlock ? GERIUM_KEY_CODE_4 : GERIUM_KEY_CODE_ARROW_LEFT};
        case kVK_ANSI_Keypad5: return { GERIUM_SCANCODE_NUMPAD_5, GERIUM_KEY_CODE_5 };
        case kVK_ANSI_Keypad6: return { GERIUM_SCANCODE_NUMPAD_6, numlock ? GERIUM_KEY_CODE_6 : GERIUM_KEY_CODE_ARROW_RIGHT };
        case kVK_ANSI_Keypad7: return { GERIUM_SCANCODE_NUMPAD_7, numlock ? GERIUM_KEY_CODE_7 : GERIUM_KEY_CODE_HOME };
        case kVK_ANSI_Keypad8: return { GERIUM_SCANCODE_NUMPAD_8, numlock ? GERIUM_KEY_CODE_8 : GERIUM_KEY_CODE_ARROW_UP };
        case kVK_ANSI_Keypad9: return { GERIUM_SCANCODE_NUMPAD_9, numlock ? GERIUM_KEY_CODE_9 : GERIUM_KEY_CODE_PAGE_UP};
    case kVK_ANSI_KeypadClear: return { GERIUM_SCANCODE_UNKNOWN, GERIUM_KEY_CODE_UNKNOWN };
        case kVK_ANSI_KeypadDecimal: return { GERIUM_SCANCODE_NUMPAD_DECIMAL, numlock ? GERIUM_KEY_CODE_PERIOD : GERIUM_KEY_CODE_DELETE };
    case kVK_ANSI_KeypadDivide: return { GERIUM_SCANCODE_NUMPAD_DEVIDE, GERIUM_KEY_CODE_DEVIDE };
    case kVK_ANSI_KeypadEnter: return { GERIUM_SCANCODE_NUMPAD_ENTER, GERIUM_KEY_CODE_ENTER };
    case kVK_ANSI_KeypadEquals: return { GERIUM_SCANCODE_NUMPAD_EQUAL, GERIUM_KEY_CODE_EQUAL };
    case kVK_ANSI_KeypadMinus: return { GERIUM_SCANCODE_NUMPAD_SUBTRACT, GERIUM_KEY_CODE_SUBTRACT };
    case kVK_ANSI_KeypadMultiply: return { GERIUM_SCANCODE_NUMPAD_MULTIPLY, GERIUM_KEY_CODE_MULTIPLY };
    case kVK_ANSI_KeypadPlus: return { GERIUM_SCANCODE_NUMPAD_ADD, GERIUM_KEY_CODE_ADD };
    case kVK_ANSI_Backslash: return { GERIUM_SCANCODE_BACKSLASH, shift ? GERIUM_KEY_CODE_PIPE : GERIUM_KEY_CODE_BACKSLASH };
    case kVK_ANSI_Comma: return { GERIUM_SCANCODE_COMMA, shift ? GERIUM_KEY_CODE_LESS : GERIUM_KEY_CODE_COMMA };
        case kVK_ANSI_Equal: return { GERIUM_SCANCODE_EQUAL, shift ? GERIUM_KEY_CODE_EQUAL : GERIUM_KEY_CODE_EQUAL };
    case kVK_ANSI_Grave: return { GERIUM_SCANCODE_BACKQUOTE, shift ? GERIUM_KEY_CODE_TILDE :  GERIUM_KEY_CODE_BACKQUOTE };
    case kVK_ANSI_LeftBracket: return { GERIUM_SCANCODE_BRACKET_LEFT, shift ? GERIUM_KEY_CODE_BRACE_LEFT : GERIUM_KEY_CODE_BRACKET_LEFT };
    case kVK_ANSI_Minus: return { GERIUM_SCANCODE_MINUS, shift ? GERIUM_KEY_CODE_UNDERSCORE : GERIUM_KEY_CODE_SUBTRACT };
    case kVK_ANSI_Period: return { GERIUM_SCANCODE_PERIOD, shift ? GERIUM_KEY_CODE_GREATER : GERIUM_KEY_CODE_PERIOD };
    case kVK_ANSI_Quote: return { GERIUM_SCANCODE_QUOTE, shift ? GERIUM_KEY_CODE_DOUBLE_QUOTE : GERIUM_KEY_CODE_QUOTE };
    case kVK_ANSI_RightBracket: return { GERIUM_SCANCODE_BRACKET_RIGHT, shift ? GERIUM_KEY_CODE_BRACE_RIGHT : GERIUM_KEY_CODE_BRACKET_RIGHT };
    case kVK_ANSI_Semicolon: return { GERIUM_SCANCODE_SEMICOLON, shift ? GERIUM_KEY_CODE_COLON : GERIUM_KEY_CODE_SEMICOLON };
    case kVK_ANSI_Slash: return { GERIUM_SCANCODE_SLASH, shift ? GERIUM_KEY_CODE_QUESTION : GERIUM_KEY_CODE_SLASH };
    case kVK_CapsLock: return { GERIUM_SCANCODE_CAPS_LOCK, GERIUM_KEY_CODE_CAPS_LOCK };
    case kVK_Command: return { GERIUM_SCANCODE_META_LEFT, GERIUM_KEY_CODE_META_LEFT };
    case kVK_Control: return { GERIUM_SCANCODE_CONTROL_LEFT, GERIUM_KEY_CODE_CONTROL_LEFT };
    case kVK_Delete: return { GERIUM_SCANCODE_BACKSPACE, GERIUM_KEY_CODE_BACKSPACE };
    case kVK_DownArrow: return { GERIUM_SCANCODE_ARROW_DOWN, GERIUM_KEY_CODE_ARROW_DOWN };
    case kVK_End: return { GERIUM_SCANCODE_END, GERIUM_KEY_CODE_END };
    case kVK_Escape: return { GERIUM_SCANCODE_ESCAPE, GERIUM_KEY_CODE_ESCAPE };
    case kVK_ForwardDelete: return { GERIUM_SCANCODE_DELETE, GERIUM_KEY_CODE_DELETE };
    case kVK_Function: return { GERIUM_SCANCODE_UNKNOWN, GERIUM_KEY_CODE_UNKNOWN };
    case kVK_Help: return { GERIUM_SCANCODE_UNKNOWN, GERIUM_KEY_CODE_UNKNOWN };
    case kVK_Home: return { GERIUM_SCANCODE_HOME, GERIUM_KEY_CODE_HOME };
    case kVK_ISO_Section: return { GERIUM_SCANCODE_INTL_BACKSLASH, GERIUM_KEY_CODE_UNKNOWN };
    case kVK_JIS_Eisu: return { GERIUM_SCANCODE_UNKNOWN, GERIUM_KEY_CODE_UNKNOWN };
    case kVK_JIS_Kana: return { GERIUM_SCANCODE_KANA_MODE, GERIUM_KEY_CODE_KANA_MODE };
    case kVK_JIS_KeypadComma: return { GERIUM_SCANCODE_NUMPAD_COMMA, GERIUM_KEY_CODE_COMMA };
    case kVK_JIS_Underscore: return { GERIUM_SCANCODE_MINUS, GERIUM_KEY_CODE_UNDERSCORE };
    case kVK_JIS_Yen: return { GERIUM_SCANCODE_INTL_YEN, GERIUM_KEY_CODE_UNKNOWN };
    case kVK_LeftArrow: return { GERIUM_SCANCODE_ARROW_LEFT, GERIUM_KEY_CODE_ARROW_LEFT };
    case kVK_Mute: return { GERIUM_SCANCODE_AUDIO_VOLUME_MUTE, GERIUM_KEY_CODE_AUDIO_VOLUME_MUTE };
    case kVK_Option: return { GERIUM_SCANCODE_ALT_LEFT, GERIUM_KEY_CODE_ALT_LEFT };
    case kVK_PageDown: return { GERIUM_SCANCODE_PAGE_DOWN, GERIUM_KEY_CODE_PAGE_DOWN };
    case kVK_PageUp: return { GERIUM_SCANCODE_PAGE_UP, GERIUM_KEY_CODE_PAGE_UP };
    case kVK_Return: return { GERIUM_SCANCODE_ENTER, GERIUM_KEY_CODE_ENTER };
    case kVK_RightArrow: return { GERIUM_SCANCODE_ARROW_RIGHT, GERIUM_KEY_CODE_ARROW_RIGHT };
    case kVK_RightCommand: return { GERIUM_SCANCODE_META_RIGHT, GERIUM_KEY_CODE_META_RIGHT };
    case kVK_RightControl: return { GERIUM_SCANCODE_CONTROL_RIGHT, GERIUM_KEY_CODE_CONTROL_RIGHT };
    case kVK_RightOption: return { GERIUM_SCANCODE_ALT_RIGHT, GERIUM_KEY_CODE_ALT_RIGHT };
    case kVK_RightShift: return { GERIUM_SCANCODE_SHIFT_RIGHT, GERIUM_KEY_CODE_SHIFT_RIGHT };
    case kVK_Shift: return { GERIUM_SCANCODE_SHIFT_LEFT, GERIUM_KEY_CODE_SHIFT_LEFT };
    case kVK_Space: return { GERIUM_SCANCODE_SPACE, GERIUM_KEY_CODE_SPACE };
    case kVK_Tab: return { GERIUM_SCANCODE_TAB, GERIUM_KEY_CODE_TAB };
    case kVK_UpArrow: return { GERIUM_SCANCODE_ARROW_UP, GERIUM_KEY_CODE_ARROW_UP };
    case kVK_VolumeDown: return { GERIUM_SCANCODE_AUDIO_VOLUME_DOWN, GERIUM_KEY_CODE_AUDIO_VOLUME_DOWN };
    case kVK_VolumeUp: return { GERIUM_SCANCODE_AUDIO_VOLUME_UP, GERIUM_KEY_CODE_AUDIO_VOLUME_UP };
        default:
            return { GERIUM_SCANCODE_UNKNOWN, GERIUM_KEY_CODE_UNKNOWN };
    }
}

gerium_inline gerium_key_mod_flags_t toModifiers(NSEventModifierFlags mods) noexcept {
    auto result = GERIUM_KEY_MOD_NONE;
    
    if (mods & NSEventModifierFlagCapsLock) {
        result |= GERIUM_KEY_MOD_CAPS_LOCK;
    }
    if (mods & NSEventModifierFlagShift) {
        result |= GERIUM_KEY_MOD_SHIFT;
    }
    if (mods & NSEventModifierFlagControl) {
        result |= GERIUM_KEY_MOD_CTRL;
    }
    if (mods & NSEventModifierFlagOption) {
        result |= GERIUM_KEY_MOD_ALT;
    }
    if (mods & NSEventModifierFlagCommand) {
        result |= GERIUM_KEY_MOD_META;
    }
    if (mods & NSEventModifierFlagNumericPad) {
        result |= GERIUM_KEY_MOD_NUM_LOCK;
    }
    
    return result;
}

} // namespace gerium::macos

#endif
