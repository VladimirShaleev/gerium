#ifndef GERIUM_ANDROID_ANDROID_SCAN_CODES_HPP
#define GERIUM_ANDROID_ANDROID_SCAN_CODES_HPP

#include "../Gerium.hpp"

namespace gerium::android {

enum class ScanCode {
    Unidentified       = 0x0000,
    Escape             = 0x0001,
    Digit1             = 0x0002,
    Digit2             = 0x0003,
    Digit3             = 0x0004,
    Digit4             = 0x0005,
    Digit5             = 0x0006,
    Digit6             = 0x0007,
    Digit7             = 0x0008,
    Digit8             = 0x0009,
    Digit9             = 0x000A,
    Digit0             = 0x000B,
    Minus              = 0x000C,
    Equal              = 0x000D,
    Backspace          = 0x000E,
    Tab                = 0x000F,
    KeyQ               = 0x0010,
    KeyW               = 0x0011,
    KeyE               = 0x0012,
    KeyR               = 0x0013,
    KeyT               = 0x0014,
    KeyY               = 0x0015,
    KeyU               = 0x0016,
    KeyI               = 0x0017,
    KeyO               = 0x0018,
    KeyP               = 0x0019,
    BracketLeft        = 0x001A,
    BracketRight       = 0x001B,
    Enter              = 0x001C,
    ControlLeft        = 0x001D,
    KeyA               = 0x001E,
    KeyS               = 0x001F,
    KeyD               = 0x0020,
    KeyF               = 0x0021,
    KeyG               = 0x0022,
    KeyH               = 0x0023,
    KeyJ               = 0x0024,
    KeyK               = 0x0025,
    KeyL               = 0x0026,
    Semicolon          = 0x0027,
    Quote              = 0x0028,
    Backquote          = 0x0029,
    ShiftLeft          = 0x002A,
    Backslash          = 0x002B,
    KeyZ               = 0x002C,
    KeyX               = 0x002D,
    KeyC               = 0x002E,
    KeyV               = 0x002F,
    KeyB               = 0x0030,
    KeyN               = 0x0031,
    KeyM               = 0x0032,
    Comma              = 0x0033,
    Period             = 0x0034,
    Slash              = 0x0035,
    ShiftRight         = 0x0036,
    NumpadMultiply     = 0x0037,
    AltLeft            = 0x0038,
    Space              = 0x0039,
    CapsLock           = 0x003A,
    F1                 = 0x003B,
    F2                 = 0x003C,
    F3                 = 0x003D,
    F4                 = 0x003E,
    F5                 = 0x003F,
    F6                 = 0x0040,
    F7                 = 0x0041,
    F8                 = 0x0042,
    F9                 = 0x0043,
    F10                = 0x0044,
    NumLock            = 0x0045,
    ScrollLock         = 0x0046,
    Numpad7            = 0x0047,
    Numpad8            = 0x0048,
    Numpad9            = 0x0049,
    NumpadSubtract     = 0x004A,
    Numpad4            = 0x004B,
    Numpad5            = 0x004C,
    Numpad6            = 0x004D,
    NumpadAdd          = 0x004E,
    Numpad1            = 0x004F,
    Numpad2            = 0x0050,
    Numpad3            = 0x0051,
    Numpad0            = 0x0052,
    NumpadDecimal      = 0x0053,
    IntlBackslash      = 0x0056,
    F11                = 0x0057,
    F12                = 0x0058,
    IntlRo             = 0x0059,
    Convert            = 0x005C,
    KanaMode           = 0x005D,
    NonConvert         = 0x005E,
    NumpadEnter        = 0x0060,
    ControlRight       = 0x0061,
    NumpadDivide       = 0x0062,
    PrintScreen        = 0x0063,
    AltRight           = 0x0064,
    Home               = 0x0066,
    ArrowUp            = 0x0067,
    PageUp             = 0x0068,
    ArrowLeft          = 0x0069,
    ArrowRight         = 0x006A,
    End                = 0x006B,
    ArrowDown          = 0x006C,
    PageDown           = 0x006D,
    Insert             = 0x006E,
    Delete             = 0x006F,
    AudioVolumeMute    = 0x0071,
    AudioVolumeDown    = 0x0072,
    AudioVolumeUp      = 0x0073,
    Power              = 0x0074,
    NumpadEqual        = 0x0075,
    Pause              = 0x0077,
    NumpadComma        = 0x0079,
    Lang1              = 0x007A,
    Lang2              = 0x007B,
    IntlYen            = 0x007C,
    MetaLeft           = 0x007D,
    MetaRight          = 0x007E,
    ContextMenu        = 0x007F,
    BrowserStop        = 0x0080,
    Again              = 0x0081,
    Props              = 0x0082,
    Undo               = 0x0083,
    Select             = 0x0084,
    Copy               = 0x0085,
    Open               = 0x0086,
    Paste              = 0x0087,
    Find               = 0x0088,
    Cut                = 0x0089,
    Help               = 0x008A,
    Sleep              = 0x008E,
    WakeUp             = 0x008F,
    LaunchApp1         = 0x0090,
    BrowserFavorites   = 0x009C,
    BrowserBack        = 0x009E,
    BrowserForward     = 0x009F,
    Eject              = 0x00A1,
    MediaTrackNext     = 0x00A3,
    MediaPlayPause     = 0x00A4,
    MediaTrackPrevious = 0x00A5,
    MediaStop          = 0x00A6,
    BrowserRefresh     = 0x00AD,
    F13                = 0x00B7,
    F14                = 0x00B8,
    F15                = 0x00B9,
    F16                = 0x00BA,
    F17                = 0x00BB,
    F18                = 0x00BC,
    F19                = 0x00BD,
    F20                = 0x00BE,
    F21                = 0x00BF,
    F22                = 0x00C0,
    F23                = 0x00C1,
    F24                = 0x00C2,
    BrowserSearch      = 0x00D9,
    Fn                 = 0x01D0
};

gerium_inline gerium_scancode_t toScanCode(int32_t scanCode) noexcept {
    switch ((ScanCode) scanCode) {
        case ScanCode::Escape:
            return GERIUM_SCANCODE_ESCAPE;
        case ScanCode::Digit1:
            return GERIUM_SCANCODE_1;
        case ScanCode::Digit2:
            return GERIUM_SCANCODE_2;
        case ScanCode::Digit3:
            return GERIUM_SCANCODE_3;
        case ScanCode::Digit4:
            return GERIUM_SCANCODE_4;
        case ScanCode::Digit5:
            return GERIUM_SCANCODE_5;
        case ScanCode::Digit6:
            return GERIUM_SCANCODE_6;
        case ScanCode::Digit7:
            return GERIUM_SCANCODE_7;
        case ScanCode::Digit8:
            return GERIUM_SCANCODE_8;
        case ScanCode::Digit9:
            return GERIUM_SCANCODE_9;
        case ScanCode::Digit0:
            return GERIUM_SCANCODE_0;
        case ScanCode::Minus:
            return GERIUM_SCANCODE_MINUS;
        case ScanCode::Equal:
            return GERIUM_SCANCODE_EQUAL;
        case ScanCode::Backspace:
            return GERIUM_SCANCODE_BACKSPACE;
        case ScanCode::Tab:
            return GERIUM_SCANCODE_TAB;
        case ScanCode::KeyQ:
            return GERIUM_SCANCODE_Q;
        case ScanCode::KeyW:
            return GERIUM_SCANCODE_W;
        case ScanCode::KeyE:
            return GERIUM_SCANCODE_E;
        case ScanCode::KeyR:
            return GERIUM_SCANCODE_R;
        case ScanCode::KeyT:
            return GERIUM_SCANCODE_T;
        case ScanCode::KeyY:
            return GERIUM_SCANCODE_Y;
        case ScanCode::KeyU:
            return GERIUM_SCANCODE_U;
        case ScanCode::KeyI:
            return GERIUM_SCANCODE_I;
        case ScanCode::KeyO:
            return GERIUM_SCANCODE_O;
        case ScanCode::KeyP:
            return GERIUM_SCANCODE_P;
        case ScanCode::BracketLeft:
            return GERIUM_SCANCODE_BRACKET_LEFT;
        case ScanCode::BracketRight:
            return GERIUM_SCANCODE_BRACKET_RIGHT;
        case ScanCode::Enter:
            return GERIUM_SCANCODE_ENTER;
        case ScanCode::ControlLeft:
            return GERIUM_SCANCODE_CONTROL_LEFT;
        case ScanCode::KeyA:
            return GERIUM_SCANCODE_A;
        case ScanCode::KeyS:
            return GERIUM_SCANCODE_S;
        case ScanCode::KeyD:
            return GERIUM_SCANCODE_D;
        case ScanCode::KeyF:
            return GERIUM_SCANCODE_F;
        case ScanCode::KeyG:
            return GERIUM_SCANCODE_G;
        case ScanCode::KeyH:
            return GERIUM_SCANCODE_H;
        case ScanCode::KeyJ:
            return GERIUM_SCANCODE_J;
        case ScanCode::KeyK:
            return GERIUM_SCANCODE_K;
        case ScanCode::KeyL:
            return GERIUM_SCANCODE_L;
        case ScanCode::Semicolon:
            return GERIUM_SCANCODE_SEMICOLON;
        case ScanCode::Quote:
            return GERIUM_SCANCODE_QUOTE;
        case ScanCode::Backquote:
            return GERIUM_SCANCODE_BACKQUOTE;
        case ScanCode::ShiftLeft:
            return GERIUM_SCANCODE_SHIFT_LEFT;
        case ScanCode::Backslash:
            return GERIUM_SCANCODE_BACKSLASH;
        case ScanCode::KeyZ:
            return GERIUM_SCANCODE_Z;
        case ScanCode::KeyX:
            return GERIUM_SCANCODE_X;
        case ScanCode::KeyC:
            return GERIUM_SCANCODE_C;
        case ScanCode::KeyV:
            return GERIUM_SCANCODE_V;
        case ScanCode::KeyB:
            return GERIUM_SCANCODE_B;
        case ScanCode::KeyN:
            return GERIUM_SCANCODE_N;
        case ScanCode::KeyM:
            return GERIUM_SCANCODE_M;
        case ScanCode::Comma:
            return GERIUM_SCANCODE_COMMA;
        case ScanCode::Period:
            return GERIUM_SCANCODE_PERIOD;
        case ScanCode::Slash:
            return GERIUM_SCANCODE_SLASH;
        case ScanCode::ShiftRight:
            return GERIUM_SCANCODE_SHIFT_RIGHT;
        case ScanCode::NumpadMultiply:
            return GERIUM_SCANCODE_NUMPAD_MULTIPLY;
        case ScanCode::AltLeft:
            return GERIUM_SCANCODE_ALT_LEFT;
        case ScanCode::Space:
            return GERIUM_SCANCODE_SPACE;
        case ScanCode::CapsLock:
            return GERIUM_SCANCODE_CAPS_LOCK;
        case ScanCode::F1:
            return GERIUM_SCANCODE_F1;
        case ScanCode::F2:
            return GERIUM_SCANCODE_F2;
        case ScanCode::F3:
            return GERIUM_SCANCODE_F3;
        case ScanCode::F4:
            return GERIUM_SCANCODE_F4;
        case ScanCode::F5:
            return GERIUM_SCANCODE_F5;
        case ScanCode::F6:
            return GERIUM_SCANCODE_F6;
        case ScanCode::F7:
            return GERIUM_SCANCODE_F7;
        case ScanCode::F8:
            return GERIUM_SCANCODE_F8;
        case ScanCode::F9:
            return GERIUM_SCANCODE_F9;
        case ScanCode::F10:
            return GERIUM_SCANCODE_F10;
        case ScanCode::NumLock:
            return GERIUM_SCANCODE_NUM_LOCK;
        case ScanCode::ScrollLock:
            return GERIUM_SCANCODE_SCROLL_LOCK;
        case ScanCode::Numpad7:
            return GERIUM_SCANCODE_NUMPAD_7;
        case ScanCode::Numpad8:
            return GERIUM_SCANCODE_NUMPAD_8;
        case ScanCode::Numpad9:
            return GERIUM_SCANCODE_NUMPAD_9;
        case ScanCode::NumpadSubtract:
            return GERIUM_SCANCODE_NUMPAD_SUBTRACT;
        case ScanCode::Numpad4:
            return GERIUM_SCANCODE_NUMPAD_4;
        case ScanCode::Numpad5:
            return GERIUM_SCANCODE_NUMPAD_5;
        case ScanCode::Numpad6:
            return GERIUM_SCANCODE_NUMPAD_6;
        case ScanCode::NumpadAdd:
            return GERIUM_SCANCODE_NUMPAD_ADD;
        case ScanCode::Numpad1:
            return GERIUM_SCANCODE_NUMPAD_1;
        case ScanCode::Numpad2:
            return GERIUM_SCANCODE_NUMPAD_2;
        case ScanCode::Numpad3:
            return GERIUM_SCANCODE_NUMPAD_3;
        case ScanCode::Numpad0:
            return GERIUM_SCANCODE_NUMPAD_0;
        case ScanCode::NumpadDecimal:
            return GERIUM_SCANCODE_NUMPAD_DECIMAL;
        case ScanCode::IntlBackslash:
            return GERIUM_SCANCODE_INTL_BACKSLASH;
        case ScanCode::F11:
            return GERIUM_SCANCODE_F11;
        case ScanCode::F12:
            return GERIUM_SCANCODE_F12;
        case ScanCode::IntlRo:
            return GERIUM_SCANCODE_INTL_RO;
        case ScanCode::Convert:
            return GERIUM_SCANCODE_CONVERT;
        case ScanCode::KanaMode:
            return GERIUM_SCANCODE_KANA_MODE;
        case ScanCode::NonConvert:
            return GERIUM_SCANCODE_NONCONVERT;
        case ScanCode::NumpadEnter:
            return GERIUM_SCANCODE_NUMPAD_ENTER;
        case ScanCode::ControlRight:
            return GERIUM_SCANCODE_CONTROL_RIGHT;
        case ScanCode::NumpadDivide:
            return GERIUM_SCANCODE_NUMPAD_DIVIDE;
        case ScanCode::PrintScreen:
            return GERIUM_SCANCODE_PRINT_SCREEN;
        case ScanCode::AltRight:
            return GERIUM_SCANCODE_ALT_RIGHT;
        case ScanCode::Home:
            return GERIUM_SCANCODE_HOME;
        case ScanCode::ArrowUp:
            return GERIUM_SCANCODE_ARROW_UP;
        case ScanCode::PageUp:
            return GERIUM_SCANCODE_PAGE_UP;
        case ScanCode::ArrowLeft:
            return GERIUM_SCANCODE_ARROW_LEFT;
        case ScanCode::ArrowRight:
            return GERIUM_SCANCODE_ARROW_RIGHT;
        case ScanCode::End:
            return GERIUM_SCANCODE_END;
        case ScanCode::ArrowDown:
            return GERIUM_SCANCODE_ARROW_DOWN;
        case ScanCode::PageDown:
            return GERIUM_SCANCODE_PAGE_DOWN;
        case ScanCode::Insert:
            return GERIUM_SCANCODE_INSERT;
        case ScanCode::Delete:
            return GERIUM_SCANCODE_DELETE;
        case ScanCode::AudioVolumeMute:
            return GERIUM_SCANCODE_AUDIO_VOLUME_MUTE;
        case ScanCode::AudioVolumeDown:
            return GERIUM_SCANCODE_AUDIO_VOLUME_DOWN;
        case ScanCode::AudioVolumeUp:
            return GERIUM_SCANCODE_AUDIO_VOLUME_UP;
        case ScanCode::Power:
            return GERIUM_SCANCODE_POWER;
        case ScanCode::NumpadEqual:
            return GERIUM_SCANCODE_NUMPAD_EQUAL;
        case ScanCode::Pause:
            return GERIUM_SCANCODE_PAUSE;
        case ScanCode::NumpadComma:
            return GERIUM_SCANCODE_NUMPAD_COMMA;
        case ScanCode::Lang1:
        case ScanCode::Lang2:
            return GERIUM_SCANCODE_UNKNOWN;
        case ScanCode::IntlYen:
            return GERIUM_SCANCODE_INTL_YEN;
        case ScanCode::MetaLeft:
            return GERIUM_SCANCODE_META_LEFT;
        case ScanCode::MetaRight:
            return GERIUM_SCANCODE_META_RIGHT;
        case ScanCode::ContextMenu:
            return GERIUM_SCANCODE_CONTEXT_MENU;
        case ScanCode::BrowserStop:
            return GERIUM_SCANCODE_BROWSER_STOP;
        case ScanCode::Again:
        case ScanCode::Props:
        case ScanCode::Undo:
        case ScanCode::Select:
        case ScanCode::Copy:
        case ScanCode::Open:
        case ScanCode::Paste:
        case ScanCode::Find:
        case ScanCode::Cut:
        case ScanCode::Help:
            return GERIUM_SCANCODE_UNKNOWN;
        case ScanCode::Sleep:
            return GERIUM_SCANCODE_SLEEP;
        case ScanCode::WakeUp:
            return GERIUM_SCANCODE_WAKE;
        case ScanCode::LaunchApp1:
            return GERIUM_SCANCODE_LAUNCH_APPLICATION_1;
        case ScanCode::BrowserFavorites:
            return GERIUM_SCANCODE_BROWSER_FAVORITES;
        case ScanCode::BrowserBack:
            return GERIUM_SCANCODE_BROWSER_BACK;
        case ScanCode::BrowserForward:
            return GERIUM_SCANCODE_BROWSER_FORWARD;
        case ScanCode::Eject:
            return GERIUM_SCANCODE_UNKNOWN;
        case ScanCode::MediaTrackNext:
            return GERIUM_SCANCODE_MEDIA_TRACK_NEXT;
        case ScanCode::MediaPlayPause:
            return GERIUM_SCANCODE_MEDIA_PLAY_PAUSE;
        case ScanCode::MediaTrackPrevious:
            return GERIUM_SCANCODE_MEDIA_TRACK_PREVIOUS;
        case ScanCode::MediaStop:
            return GERIUM_SCANCODE_MEDIA_STOP;
        case ScanCode::BrowserRefresh:
            return GERIUM_SCANCODE_BROWSER_REFRESH;
        case ScanCode::F13:
            return GERIUM_SCANCODE_F13;
        case ScanCode::F14:
            return GERIUM_SCANCODE_F14;
        case ScanCode::F15:
            return GERIUM_SCANCODE_F15;
        case ScanCode::F16:
            return GERIUM_SCANCODE_F16;
        case ScanCode::F17:
            return GERIUM_SCANCODE_F17;
        case ScanCode::F18:
            return GERIUM_SCANCODE_F18;
        case ScanCode::F19:
            return GERIUM_SCANCODE_F19;
        case ScanCode::F20:
            return GERIUM_SCANCODE_F20;
        case ScanCode::F21:
            return GERIUM_SCANCODE_F21;
        case ScanCode::F22:
            return GERIUM_SCANCODE_F22;
        case ScanCode::F23:
            return GERIUM_SCANCODE_F23;
        case ScanCode::F24:
            return GERIUM_SCANCODE_F24;
        case ScanCode::BrowserSearch:
            return GERIUM_SCANCODE_BROWSER_SEARCH;
        case ScanCode::Fn:
        default:
            return GERIUM_SCANCODE_UNKNOWN;
    }
}

gerium_inline gerium_key_code_t toKeyCode(int32_t keycode) {
    switch (keycode) {
        case AKEYCODE_SOFT_LEFT:
        case AKEYCODE_SOFT_RIGHT:
        case AKEYCODE_HOME:
        case AKEYCODE_BACK:
        case AKEYCODE_CALL:
        case AKEYCODE_ENDCALL:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_0:
            return GERIUM_KEY_CODE_0;
        case AKEYCODE_1:
            return GERIUM_KEY_CODE_1;
        case AKEYCODE_2:
            return GERIUM_KEY_CODE_2;
        case AKEYCODE_3:
            return GERIUM_KEY_CODE_3;
        case AKEYCODE_4:
            return GERIUM_KEY_CODE_4;
        case AKEYCODE_5:
            return GERIUM_KEY_CODE_5;
        case AKEYCODE_6:
            return GERIUM_KEY_CODE_6;
        case AKEYCODE_7:
            return GERIUM_KEY_CODE_7;
        case AKEYCODE_8:
            return GERIUM_KEY_CODE_8;
        case AKEYCODE_9:
            return GERIUM_KEY_CODE_9;
        case AKEYCODE_STAR:
            return GERIUM_KEY_CODE_ASTERISK;
        case AKEYCODE_POUND:
            return GERIUM_KEY_CODE_HASH;
        case AKEYCODE_DPAD_UP:
            return GERIUM_KEY_CODE_ARROW_UP;
        case AKEYCODE_DPAD_DOWN:
            return GERIUM_KEY_CODE_ARROW_DOWN;
        case AKEYCODE_DPAD_LEFT:
            return GERIUM_KEY_CODE_ARROW_LEFT;
        case AKEYCODE_DPAD_RIGHT:
            return GERIUM_KEY_CODE_ARROW_RIGHT;
        case AKEYCODE_DPAD_CENTER:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_VOLUME_UP:
            return GERIUM_KEY_CODE_AUDIO_VOLUME_UP;
        case AKEYCODE_VOLUME_DOWN:
            return GERIUM_KEY_CODE_AUDIO_VOLUME_DOWN;
        case AKEYCODE_POWER:
            return GERIUM_KEY_CODE_POWER;
        case AKEYCODE_CAMERA:
        case AKEYCODE_CLEAR:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_A:
            return GERIUM_KEY_CODE_A;
        case AKEYCODE_B:
            return GERIUM_KEY_CODE_B;
        case AKEYCODE_C:
            return GERIUM_KEY_CODE_C;
        case AKEYCODE_D:
            return GERIUM_KEY_CODE_D;
        case AKEYCODE_E:
            return GERIUM_KEY_CODE_E;
        case AKEYCODE_F:
            return GERIUM_KEY_CODE_F;
        case AKEYCODE_G:
            return GERIUM_KEY_CODE_G;
        case AKEYCODE_H:
            return GERIUM_KEY_CODE_H;
        case AKEYCODE_I:
            return GERIUM_KEY_CODE_I;
        case AKEYCODE_J:
            return GERIUM_KEY_CODE_J;
        case AKEYCODE_K:
            return GERIUM_KEY_CODE_K;
        case AKEYCODE_L:
            return GERIUM_KEY_CODE_L;
        case AKEYCODE_M:
            return GERIUM_KEY_CODE_M;
        case AKEYCODE_N:
            return GERIUM_KEY_CODE_N;
        case AKEYCODE_O:
            return GERIUM_KEY_CODE_O;
        case AKEYCODE_P:
            return GERIUM_KEY_CODE_P;
        case AKEYCODE_Q:
            return GERIUM_KEY_CODE_Q;
        case AKEYCODE_R:
            return GERIUM_KEY_CODE_R;
        case AKEYCODE_S:
            return GERIUM_KEY_CODE_S;
        case AKEYCODE_T:
            return GERIUM_KEY_CODE_T;
        case AKEYCODE_U:
            return GERIUM_KEY_CODE_U;
        case AKEYCODE_V:
            return GERIUM_KEY_CODE_V;
        case AKEYCODE_W:
            return GERIUM_KEY_CODE_W;
        case AKEYCODE_X:
            return GERIUM_KEY_CODE_X;
        case AKEYCODE_Y:
            return GERIUM_KEY_CODE_Y;
        case AKEYCODE_Z:
            return GERIUM_KEY_CODE_Z;
        case AKEYCODE_COMMA:
            return GERIUM_KEY_CODE_COMMA;
        case AKEYCODE_PERIOD:
            return GERIUM_KEY_CODE_PERIOD;
        case AKEYCODE_ALT_LEFT:
            return GERIUM_KEY_CODE_ALT_LEFT;
        case AKEYCODE_ALT_RIGHT:
            return GERIUM_KEY_CODE_ALT_RIGHT;
        case AKEYCODE_SHIFT_LEFT:
            return GERIUM_KEY_CODE_SHIFT_LEFT;
        case AKEYCODE_SHIFT_RIGHT:
            return GERIUM_KEY_CODE_SHIFT_RIGHT;
        case AKEYCODE_TAB:
            return GERIUM_KEY_CODE_TAB;
        case AKEYCODE_SPACE:
            return GERIUM_KEY_CODE_SPACE;
        case AKEYCODE_SYM:
        case AKEYCODE_EXPLORER:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_ENVELOPE:
            return GERIUM_KEY_CODE_LAUNCH_MAIL;
        case AKEYCODE_ENTER:
            return GERIUM_KEY_CODE_ENTER;
        case AKEYCODE_DEL:
            return GERIUM_KEY_CODE_BACKSPACE;
        case AKEYCODE_GRAVE:
            return GERIUM_KEY_CODE_BACKQUOTE;
        case AKEYCODE_MINUS:
            return GERIUM_KEY_CODE_SUBTRACT;
        case AKEYCODE_EQUALS:
            return GERIUM_KEY_CODE_EQUAL;
        case AKEYCODE_LEFT_BRACKET:
            return GERIUM_KEY_CODE_BRACE_LEFT;
        case AKEYCODE_RIGHT_BRACKET:
            return GERIUM_KEY_CODE_BRACE_RIGHT;
        case AKEYCODE_BACKSLASH:
            return GERIUM_KEY_CODE_BACKSLASH;
        case AKEYCODE_SEMICOLON:
            return GERIUM_KEY_CODE_SEMICOLON;
        case AKEYCODE_APOSTROPHE:
            return GERIUM_KEY_CODE_QUOTE;
        case AKEYCODE_SLASH:
            return GERIUM_KEY_CODE_SLASH;
        case AKEYCODE_AT:
            return GERIUM_KEY_CODE_AT;
        case AKEYCODE_NUM:
        case AKEYCODE_HEADSETHOOK:
        case AKEYCODE_FOCUS:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_PLUS:
            return GERIUM_KEY_CODE_ADD;
        case AKEYCODE_MENU:
            return GERIUM_KEY_CODE_CONTEXT_MENU;
        case AKEYCODE_NOTIFICATION:
        case AKEYCODE_SEARCH:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_MEDIA_PLAY_PAUSE:
            return GERIUM_KEY_CODE_MEDIA_PLAY_PAUSE;
        case AKEYCODE_MEDIA_STOP:
            return GERIUM_KEY_CODE_MEDIA_STOP;
        case AKEYCODE_MEDIA_NEXT:
            return GERIUM_KEY_CODE_MEDIA_TRACK_NEXT;
        case AKEYCODE_MEDIA_PREVIOUS:
            return GERIUM_KEY_CODE_MEDIA_TRACK_PREVIOUS;
        case AKEYCODE_MEDIA_REWIND:
        case AKEYCODE_MEDIA_FAST_FORWARD:
        case AKEYCODE_MUTE:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_PAGE_UP:
            return GERIUM_KEY_CODE_PAGE_UP;
        case AKEYCODE_PAGE_DOWN:
            return GERIUM_KEY_CODE_PAGE_DOWN;
        case AKEYCODE_PICTSYMBOLS:
        case AKEYCODE_SWITCH_CHARSET:
        case AKEYCODE_BUTTON_A:
        case AKEYCODE_BUTTON_B:
        case AKEYCODE_BUTTON_C:
        case AKEYCODE_BUTTON_X:
        case AKEYCODE_BUTTON_Y:
        case AKEYCODE_BUTTON_Z:
        case AKEYCODE_BUTTON_L1:
        case AKEYCODE_BUTTON_R1:
        case AKEYCODE_BUTTON_L2:
        case AKEYCODE_BUTTON_R2:
        case AKEYCODE_BUTTON_THUMBL:
        case AKEYCODE_BUTTON_THUMBR:
        case AKEYCODE_BUTTON_START:
        case AKEYCODE_BUTTON_SELECT:
        case AKEYCODE_BUTTON_MODE:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_ESCAPE:
            return GERIUM_KEY_CODE_ESCAPE;
        case AKEYCODE_FORWARD_DEL:
            return GERIUM_KEY_CODE_DELETE;
        case AKEYCODE_CTRL_LEFT:
            return GERIUM_KEY_CODE_CONTROL_LEFT;
        case AKEYCODE_CTRL_RIGHT:
            return GERIUM_KEY_CODE_CONTROL_RIGHT;
        case AKEYCODE_CAPS_LOCK:
            return GERIUM_KEY_CODE_CAPS_LOCK;
        case AKEYCODE_SCROLL_LOCK:
            return GERIUM_KEY_CODE_SCROLL_LOCK;
        case AKEYCODE_META_LEFT:
            return GERIUM_KEY_CODE_META_LEFT;
        case AKEYCODE_META_RIGHT:
            return GERIUM_KEY_CODE_META_RIGHT;
        case AKEYCODE_FUNCTION:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_SYSRQ:
            return GERIUM_KEY_CODE_PRINT_SCREEN;
        case AKEYCODE_BREAK:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_MOVE_HOME:
            return GERIUM_KEY_CODE_HOME;
        case AKEYCODE_MOVE_END:
            return GERIUM_KEY_CODE_END;
        case AKEYCODE_INSERT:
            return GERIUM_KEY_CODE_INSERT;
        case AKEYCODE_FORWARD:
        case AKEYCODE_MEDIA_PLAY:
        case AKEYCODE_MEDIA_PAUSE:
        case AKEYCODE_MEDIA_CLOSE:
        case AKEYCODE_MEDIA_EJECT:
        case AKEYCODE_MEDIA_RECORD:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_F1:
            return GERIUM_KEY_CODE_F1;
        case AKEYCODE_F2:
            return GERIUM_KEY_CODE_F2;
        case AKEYCODE_F3:
            return GERIUM_KEY_CODE_F3;
        case AKEYCODE_F4:
            return GERIUM_KEY_CODE_F4;
        case AKEYCODE_F5:
            return GERIUM_KEY_CODE_F5;
        case AKEYCODE_F6:
            return GERIUM_KEY_CODE_F6;
        case AKEYCODE_F7:
            return GERIUM_KEY_CODE_F7;
        case AKEYCODE_F8:
            return GERIUM_KEY_CODE_F8;
        case AKEYCODE_F9:
            return GERIUM_KEY_CODE_F9;
        case AKEYCODE_F10:
            return GERIUM_KEY_CODE_F10;
        case AKEYCODE_F11:
            return GERIUM_KEY_CODE_F11;
        case AKEYCODE_F12:
            return GERIUM_KEY_CODE_F12;
        case AKEYCODE_NUM_LOCK:
            return GERIUM_KEY_CODE_NUM_LOCK;
        case AKEYCODE_NUMPAD_0:
            return GERIUM_KEY_CODE_NUMPAD_0;
        case AKEYCODE_NUMPAD_1:
            return GERIUM_KEY_CODE_NUMPAD_1;
        case AKEYCODE_NUMPAD_2:
            return GERIUM_KEY_CODE_NUMPAD_2;
        case AKEYCODE_NUMPAD_3:
            return GERIUM_KEY_CODE_NUMPAD_3;
        case AKEYCODE_NUMPAD_4:
            return GERIUM_KEY_CODE_NUMPAD_4;
        case AKEYCODE_NUMPAD_5:
            return GERIUM_KEY_CODE_NUMPAD_5;
        case AKEYCODE_NUMPAD_6:
            return GERIUM_KEY_CODE_NUMPAD_6;
        case AKEYCODE_NUMPAD_7:
            return GERIUM_KEY_CODE_NUMPAD_7;
        case AKEYCODE_NUMPAD_8:
            return GERIUM_KEY_CODE_NUMPAD_8;
        case AKEYCODE_NUMPAD_9:
            return GERIUM_KEY_CODE_NUMPAD_9;
        case AKEYCODE_NUMPAD_DIVIDE:
            return GERIUM_KEY_CODE_DIVIDE;
        case AKEYCODE_NUMPAD_MULTIPLY:
            return GERIUM_KEY_CODE_MULTIPLY;
        case AKEYCODE_NUMPAD_SUBTRACT:
            return GERIUM_KEY_CODE_SUBTRACT;
        case AKEYCODE_NUMPAD_ADD:
            return GERIUM_KEY_CODE_ADD;
        case AKEYCODE_NUMPAD_DOT:
            return GERIUM_KEY_CODE_PERIOD;
        case AKEYCODE_NUMPAD_COMMA:
            return GERIUM_KEY_CODE_COMMA;
        case AKEYCODE_NUMPAD_ENTER:
            return GERIUM_KEY_CODE_ENTER;
        case AKEYCODE_NUMPAD_EQUALS:
            return GERIUM_KEY_CODE_EQUAL;
        case AKEYCODE_NUMPAD_LEFT_PAREN:
            return GERIUM_KEY_CODE_PAREN_LEFT;
        case AKEYCODE_NUMPAD_RIGHT_PAREN:
            return GERIUM_KEY_CODE_PAREN_RIGHT;
        case AKEYCODE_VOLUME_MUTE:
            return GERIUM_KEY_CODE_AUDIO_VOLUME_MUTE;
        case AKEYCODE_INFO:
        case AKEYCODE_CHANNEL_UP:
        case AKEYCODE_CHANNEL_DOWN:
        case AKEYCODE_ZOOM_IN:
        case AKEYCODE_ZOOM_OUT:
        case AKEYCODE_TV:
        case AKEYCODE_WINDOW:
        case AKEYCODE_GUIDE:
        case AKEYCODE_DVR:
        case AKEYCODE_BOOKMARK:
        case AKEYCODE_CAPTIONS:
        case AKEYCODE_SETTINGS:
        case AKEYCODE_TV_POWER:
        case AKEYCODE_TV_INPUT:
        case AKEYCODE_STB_POWER:
        case AKEYCODE_STB_INPUT:
        case AKEYCODE_AVR_POWER:
        case AKEYCODE_AVR_INPUT:
        case AKEYCODE_PROG_RED:
        case AKEYCODE_PROG_GREEN:
        case AKEYCODE_PROG_YELLOW:
        case AKEYCODE_PROG_BLUE:
        case AKEYCODE_APP_SWITCH:
        case AKEYCODE_BUTTON_1:
        case AKEYCODE_BUTTON_2:
        case AKEYCODE_BUTTON_3:
        case AKEYCODE_BUTTON_4:
        case AKEYCODE_BUTTON_5:
        case AKEYCODE_BUTTON_6:
        case AKEYCODE_BUTTON_7:
        case AKEYCODE_BUTTON_8:
        case AKEYCODE_BUTTON_9:
        case AKEYCODE_BUTTON_10:
        case AKEYCODE_BUTTON_11:
        case AKEYCODE_BUTTON_12:
        case AKEYCODE_BUTTON_13:
        case AKEYCODE_BUTTON_14:
        case AKEYCODE_BUTTON_15:
        case AKEYCODE_BUTTON_16:
        case AKEYCODE_LANGUAGE_SWITCH:
        case AKEYCODE_MANNER_MODE:
        case AKEYCODE_3D_MODE:
        case AKEYCODE_CONTACTS:
        case AKEYCODE_CALENDAR:
        case AKEYCODE_MUSIC:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_CALCULATOR:
            return GERIUM_KEY_CODE_LAUNCH_APPLICATION_1;
        case AKEYCODE_ZENKAKU_HANKAKU:
        case AKEYCODE_EISU:
        case AKEYCODE_MUHENKAN:
        case AKEYCODE_HENKAN:
        case AKEYCODE_KATAKANA_HIRAGANA:
        case AKEYCODE_YEN:
        case AKEYCODE_RO:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_KANA:
            return GERIUM_KEY_CODE_KANA_MODE;
        case AKEYCODE_ASSIST:
        case AKEYCODE_BRIGHTNESS_DOWN:
        case AKEYCODE_BRIGHTNESS_UP:
        case AKEYCODE_MEDIA_AUDIO_TRACK:
            return GERIUM_KEY_CODE_UNKNOWN;
        case AKEYCODE_SLEEP:
            return GERIUM_KEY_CODE_SLEEP;
        case AKEYCODE_WAKEUP:
            return GERIUM_KEY_CODE_WAKE;
        case AKEYCODE_PAIRING:
        case AKEYCODE_MEDIA_TOP_MENU:
        case AKEYCODE_11:
        case AKEYCODE_12:
        case AKEYCODE_LAST_CHANNEL:
        case AKEYCODE_TV_DATA_SERVICE:
        case AKEYCODE_VOICE_ASSIST:
        case AKEYCODE_TV_RADIO_SERVICE:
        case AKEYCODE_TV_TELETEXT:
        case AKEYCODE_TV_NUMBER_ENTRY:
        case AKEYCODE_TV_TERRESTRIAL_ANALOG:
        case AKEYCODE_TV_TERRESTRIAL_DIGITAL:
        case AKEYCODE_TV_SATELLITE:
        case AKEYCODE_TV_SATELLITE_BS:
        case AKEYCODE_TV_SATELLITE_CS:
        case AKEYCODE_TV_SATELLITE_SERVICE:
        case AKEYCODE_TV_NETWORK:
        case AKEYCODE_TV_ANTENNA_CABLE:
        case AKEYCODE_TV_INPUT_HDMI_1:
        case AKEYCODE_TV_INPUT_HDMI_2:
        case AKEYCODE_TV_INPUT_HDMI_3:
        case AKEYCODE_TV_INPUT_HDMI_4:
        case AKEYCODE_TV_INPUT_COMPOSITE_1:
        case AKEYCODE_TV_INPUT_COMPOSITE_2:
        case AKEYCODE_TV_INPUT_COMPONENT_1:
        case AKEYCODE_TV_INPUT_COMPONENT_2:
        case AKEYCODE_TV_INPUT_VGA_1:
        case AKEYCODE_TV_AUDIO_DESCRIPTION:
        case AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_UP:
        case AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_DOWN:
        case AKEYCODE_TV_ZOOM_MODE:
        case AKEYCODE_TV_CONTENTS_MENU:
        case AKEYCODE_TV_MEDIA_CONTEXT_MENU:
        case AKEYCODE_TV_TIMER_PROGRAMMING:
        case AKEYCODE_HELP:
        case AKEYCODE_NAVIGATE_PREVIOUS:
        case AKEYCODE_NAVIGATE_NEXT:
        case AKEYCODE_NAVIGATE_IN:
        case AKEYCODE_NAVIGATE_OUT:
        case AKEYCODE_STEM_PRIMARY:
        case AKEYCODE_STEM_1:
        case AKEYCODE_STEM_2:
        case AKEYCODE_STEM_3:
        case AKEYCODE_DPAD_UP_LEFT:
        case AKEYCODE_DPAD_DOWN_LEFT:
        case AKEYCODE_DPAD_UP_RIGHT:
        case AKEYCODE_DPAD_DOWN_RIGHT:
        case AKEYCODE_MEDIA_SKIP_FORWARD:
        case AKEYCODE_MEDIA_SKIP_BACKWARD:
        case AKEYCODE_MEDIA_STEP_FORWARD:
        case AKEYCODE_MEDIA_STEP_BACKWARD:
        case AKEYCODE_SOFT_SLEEP:
        case AKEYCODE_CUT:
        case AKEYCODE_COPY:
        case AKEYCODE_PASTE:
        case AKEYCODE_SYSTEM_NAVIGATION_UP:
        case AKEYCODE_SYSTEM_NAVIGATION_DOWN:
        case AKEYCODE_SYSTEM_NAVIGATION_LEFT:
        case AKEYCODE_SYSTEM_NAVIGATION_RIGHT:
        case AKEYCODE_ALL_APPS:
        case AKEYCODE_REFRESH:
        case AKEYCODE_THUMBS_UP:
        case AKEYCODE_THUMBS_DOWN:
        case AKEYCODE_PROFILE_SWITCH:
        case AKEYCODE_VIDEO_APP_1:
        case AKEYCODE_VIDEO_APP_2:
        case AKEYCODE_VIDEO_APP_3:
        case AKEYCODE_VIDEO_APP_4:
        case AKEYCODE_VIDEO_APP_5:
        case AKEYCODE_VIDEO_APP_6:
        case AKEYCODE_VIDEO_APP_7:
        case AKEYCODE_VIDEO_APP_8:
        case AKEYCODE_FEATURED_APP_1:
        case AKEYCODE_FEATURED_APP_2:
        case AKEYCODE_FEATURED_APP_3:
        case AKEYCODE_FEATURED_APP_4:
        case AKEYCODE_DEMO_APP_1:
        case AKEYCODE_DEMO_APP_2:
        case AKEYCODE_DEMO_APP_3:
        case AKEYCODE_DEMO_APP_4:
        default:
            return GERIUM_KEY_CODE_UNKNOWN;
    }
}

gerium_inline gerium_key_mod_flags_t toModifiers(int32_t meta) noexcept {
    auto modifiers = GERIUM_KEY_MOD_NONE;

    if (meta & AMETA_ALT_LEFT_ON) {
        modifiers |= GERIUM_KEY_MOD_LALT;
    }
    if (meta & AMETA_ALT_RIGHT_ON) {
        modifiers |= GERIUM_KEY_MOD_RALT;
    }
    if (meta & AMETA_SHIFT_LEFT_ON) {
        modifiers |= GERIUM_KEY_MOD_LSHIFT;
    }
    if (meta & AMETA_SHIFT_RIGHT_ON) {
        modifiers |= GERIUM_KEY_MOD_RSHIFT;
    }
    if (meta & AMETA_CTRL_LEFT_ON) {
        modifiers |= GERIUM_KEY_MOD_LCTRL;
    }
    if (meta & AMETA_CTRL_RIGHT_ON) {
        modifiers |= GERIUM_KEY_MOD_RCTRL;
    }
    if (meta & AMETA_META_LEFT_ON) {
        modifiers |= GERIUM_KEY_MOD_LMETA;
    }
    if (meta & AMETA_META_RIGHT_ON) {
        modifiers |= GERIUM_KEY_MOD_RMETA;
    }
    if (meta & AMETA_CAPS_LOCK_ON) {
        modifiers |= GERIUM_KEY_MOD_CAPS_LOCK;
    }
    if (meta & AMETA_NUM_LOCK_ON) {
        modifiers |= GERIUM_KEY_MOD_NUM_LOCK;
    }
    if (meta & AMETA_SCROLL_LOCK_ON) {
        modifiers |= GERIUM_KEY_MOD_SCROLL_LOCK;
    }

    return modifiers;
}

} // namespace gerium::android

#endif
