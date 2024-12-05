#ifndef GERIUM_LINUX_LINUX_SCAN_CODES_HPP
#define GERIUM_LINUX_LINUX_SCAN_CODES_HPP

#include "../Gerium.hpp"

namespace gerium::linux {

enum class ScanCode {
    Escape             = 0x0009,
    Digit1             = 0x000A,
    Digit2             = 0x000B,
    Digit3             = 0x000C,
    Digit4             = 0x000D,
    Digit5             = 0x000E,
    Digit6             = 0x000F,
    Digit7             = 0x0010,
    Digit8             = 0x0011,
    Digit9             = 0x0012,
    Digit0             = 0x0013,
    Minus              = 0x0014,
    Equal              = 0x0015,
    Backspace          = 0x0016,
    Tab                = 0x0017,
    KeyQ               = 0x0018,
    KeyW               = 0x0019,
    KeyE               = 0x001A,
    KeyR               = 0x001B,
    KeyT               = 0x001C,
    KeyY               = 0x001D,
    KeyU               = 0x001E,
    KeyI               = 0x001F,
    KeyO               = 0x0020,
    KeyP               = 0x0021,
    BracketLeft        = 0x0022,
    BracketRight       = 0x0023,
    Enter              = 0x0024,
    ControlLeft        = 0x0025,
    KeyA               = 0x0026,
    KeyS               = 0x0027,
    KeyD               = 0x0028,
    KeyF               = 0x0029,
    KeyG               = 0x002A,
    KeyH               = 0x002B,
    KeyJ               = 0x002C,
    KeyK               = 0x002D,
    KeyL               = 0x002E,
    Semicolon          = 0x002F,
    Quote              = 0x0030,
    Backquote          = 0x0031,
    ShiftLeft          = 0x0032,
    Backslash          = 0x0033,
    KeyZ               = 0x0034,
    KeyX               = 0x0035,
    KeyC               = 0x0036,
    KeyV               = 0x0037,
    KeyB               = 0x0038,
    KeyN               = 0x0039,
    KeyM               = 0x003A,
    Comma              = 0x003B,
    Period             = 0x003C,
    Slash              = 0x003D,
    ShiftRight         = 0x003E,
    NumpadMultiply     = 0x003F,
    AltLeft            = 0x0040,
    Space              = 0x0041,
    CapsLock           = 0x0042,
    F1                 = 0x0043,
    F2                 = 0x0044,
    F3                 = 0x0045,
    F4                 = 0x0046,
    F5                 = 0x0047,
    F6                 = 0x0048,
    F7                 = 0x0049,
    F8                 = 0x004A,
    F9                 = 0x004B,
    F10                = 0x004C,
    NumLock            = 0x004D,
    ScrollLock         = 0x004E,
    Numpad7            = 0x004F,
    Numpad8            = 0x0050,
    Numpad9            = 0x0051,
    NumpadSubtract     = 0x0052,
    Numpad4            = 0x0053,
    Numpad5            = 0x0054,
    Numpad6            = 0x0055,
    NumpadAdd          = 0x0056,
    Numpad1            = 0x0057,
    Numpad2            = 0x0058,
    Numpad3            = 0x0059,
    Numpad0            = 0x005A,
    NumpadDecimal      = 0x005B,
    IntlBackslash      = 0x005E,
    F11                = 0x005F,
    F12                = 0x0060,
    IntlRo             = 0x0061,
    Convert            = 0x0064,
    KanaMode           = 0x0065,
    NonConvert         = 0x0066,
    NumpadEnter        = 0x0068,
    ControlRight       = 0x0069,
    NumpadDivide       = 0x006A,
    PrintScreen        = 0x006B,
    AltRight           = 0x006C,
    Home               = 0x006E,
    ArrowUp            = 0x006F,
    PageUp             = 0x0070,
    ArrowLeft          = 0x0071,
    ArrowRight         = 0x0072,
    End                = 0x0073,
    ArrowDown          = 0x0074,
    PageDown           = 0x0075,
    Insert             = 0x0076,
    Delete             = 0x0077,
    AudioVolumeMute    = 0x0079,
    AudioVolumeDown    = 0x007A,
    AudioVolumeUp      = 0x007B,
    NumpadEqual        = 0x007D,
    Pause              = 0x007F,
    NumpadComma        = 0x0081,
    Lang1              = 0x0082,
    Lang2              = 0x0083,
    IntlYen            = 0x0084,
    MetaLeft           = 0x0085,
    MetaRight          = 0x0086,
    ContextMenu        = 0x0087,
    BrowserStop        = 0x0088,
    Again              = 0x0089,
    Undo               = 0x008B,
    Select             = 0x008C,
    Copy               = 0x008D,
    Open               = 0x008E,
    Paste              = 0x008F,
    Find               = 0x0090,
    Cut                = 0x0091,
    Help               = 0x0092,
    LaunchApp2         = 0x0094,
    Sleep              = 0x0096,
    WakeUp             = 0x0097,
    LaunchApp1         = 0x0098,
    LaunchMail         = 0x00A3,
    BrowserFavorites   = 0x00A4,
    BrowserBack        = 0x00A6,
    BrowserForward     = 0x00A7,
    Eject              = 0x00A9,
    MediaTrackNext     = 0x00AB,
    MediaPlayPause     = 0x00AC,
    MediaTrackPrevious = 0x00AD,
    MediaStop          = 0x00AE,
    MediaSelect        = 0x00B3,
    BrowserHome        = 0x00B4,
    BrowserRefresh     = 0x00B5,
    F13                = 0x00BF,
    F14                = 0x00C0,
    F15                = 0x00C1,
    F16                = 0x00C2,
    F17                = 0x00C3,
    F18                = 0x00C4,
    F19                = 0x00C5,
    F20                = 0x00C6,
    F21                = 0x00C7,
    F22                = 0x00C8,
    F23                = 0x00C9,
    F24                = 0x00CA,
    BrowserSearch      = 0x00E1
};

gerium_inline gerium_scancode_t toScanCode(ScanCode scanCode) noexcept {
    switch (scanCode) {
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
        case ScanCode::Undo:
        case ScanCode::Select:
        case ScanCode::Copy:
        case ScanCode::Open:
        case ScanCode::Paste:
        case ScanCode::Find:
        case ScanCode::Cut:
        case ScanCode::Help:
            return GERIUM_SCANCODE_UNKNOWN;
        case ScanCode::LaunchApp2:
            return GERIUM_SCANCODE_LAUNCH_APPLICATION_2;
        case ScanCode::Sleep:
            return GERIUM_SCANCODE_SLEEP;
        case ScanCode::WakeUp:
            return GERIUM_SCANCODE_WAKE;
        case ScanCode::LaunchApp1:
            return GERIUM_SCANCODE_LAUNCH_APPLICATION_1;
        case ScanCode::LaunchMail:
            return GERIUM_SCANCODE_LAUNCH_MAIL;
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
        case ScanCode::MediaSelect:
            return GERIUM_SCANCODE_UNKNOWN;
        case ScanCode::BrowserHome:
            return GERIUM_SCANCODE_BROWSER_HOME;
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
        default:
            return GERIUM_SCANCODE_UNKNOWN;
    }
}

gerium_inline gerium_key_code_t toKeyCode(gerium_scancode_t scanCode, bool shift, bool numlock) noexcept {
    switch (scanCode) {
        case GERIUM_SCANCODE_UNKNOWN:
            return GERIUM_KEY_CODE_UNKNOWN;
        case GERIUM_SCANCODE_0:
            return shift ? GERIUM_KEY_CODE_PAREN_RIGHT : GERIUM_KEY_CODE_0;
        case GERIUM_SCANCODE_1:
            return shift ? GERIUM_KEY_CODE_EXCLAIM : GERIUM_KEY_CODE_1;
        case GERIUM_SCANCODE_2:
            return shift ? GERIUM_KEY_CODE_AT : GERIUM_KEY_CODE_2;
        case GERIUM_SCANCODE_3:
            return shift ? GERIUM_KEY_CODE_HASH : GERIUM_KEY_CODE_3;
        case GERIUM_SCANCODE_4:
            return shift ? GERIUM_KEY_CODE_DOLLAR : GERIUM_KEY_CODE_4;
        case GERIUM_SCANCODE_5:
            return shift ? GERIUM_KEY_CODE_PERCENT : GERIUM_KEY_CODE_5;
        case GERIUM_SCANCODE_6:
            return shift ? GERIUM_KEY_CODE_CARET : GERIUM_KEY_CODE_6;
        case GERIUM_SCANCODE_7:
            return shift ? GERIUM_KEY_CODE_AMPERSAND : GERIUM_KEY_CODE_7;
        case GERIUM_SCANCODE_8:
            return shift ? GERIUM_KEY_CODE_ASTERISK : GERIUM_KEY_CODE_8;
        case GERIUM_SCANCODE_9:
            return shift ? GERIUM_KEY_CODE_PAREN_LEFT : GERIUM_KEY_CODE_9;
        case GERIUM_SCANCODE_A:
            return GERIUM_KEY_CODE_A;
        case GERIUM_SCANCODE_B:
            return GERIUM_KEY_CODE_B;
        case GERIUM_SCANCODE_C:
            return GERIUM_KEY_CODE_C;
        case GERIUM_SCANCODE_D:
            return GERIUM_KEY_CODE_D;
        case GERIUM_SCANCODE_E:
            return GERIUM_KEY_CODE_E;
        case GERIUM_SCANCODE_F:
            return GERIUM_KEY_CODE_F;
        case GERIUM_SCANCODE_G:
            return GERIUM_KEY_CODE_G;
        case GERIUM_SCANCODE_H:
            return GERIUM_KEY_CODE_H;
        case GERIUM_SCANCODE_I:
            return GERIUM_KEY_CODE_I;
        case GERIUM_SCANCODE_J:
            return GERIUM_KEY_CODE_J;
        case GERIUM_SCANCODE_K:
            return GERIUM_KEY_CODE_K;
        case GERIUM_SCANCODE_L:
            return GERIUM_KEY_CODE_L;
        case GERIUM_SCANCODE_M:
            return GERIUM_KEY_CODE_M;
        case GERIUM_SCANCODE_N:
            return GERIUM_KEY_CODE_N;
        case GERIUM_SCANCODE_O:
            return GERIUM_KEY_CODE_O;
        case GERIUM_SCANCODE_P:
            return GERIUM_KEY_CODE_P;
        case GERIUM_SCANCODE_Q:
            return GERIUM_KEY_CODE_Q;
        case GERIUM_SCANCODE_R:
            return GERIUM_KEY_CODE_R;
        case GERIUM_SCANCODE_S:
            return GERIUM_KEY_CODE_S;
        case GERIUM_SCANCODE_T:
            return GERIUM_KEY_CODE_T;
        case GERIUM_SCANCODE_U:
            return GERIUM_KEY_CODE_U;
        case GERIUM_SCANCODE_V:
            return GERIUM_KEY_CODE_V;
        case GERIUM_SCANCODE_W:
            return GERIUM_KEY_CODE_W;
        case GERIUM_SCANCODE_X:
            return GERIUM_KEY_CODE_X;
        case GERIUM_SCANCODE_Y:
            return GERIUM_KEY_CODE_Y;
        case GERIUM_SCANCODE_Z:
            return GERIUM_KEY_CODE_Z;
        case GERIUM_SCANCODE_F1:
            return GERIUM_KEY_CODE_F1;
        case GERIUM_SCANCODE_F2:
            return GERIUM_KEY_CODE_F2;
        case GERIUM_SCANCODE_F3:
            return GERIUM_KEY_CODE_F3;
        case GERIUM_SCANCODE_F4:
            return GERIUM_KEY_CODE_F4;
        case GERIUM_SCANCODE_F5:
            return GERIUM_KEY_CODE_F5;
        case GERIUM_SCANCODE_F6:
            return GERIUM_KEY_CODE_F6;
        case GERIUM_SCANCODE_F7:
            return GERIUM_KEY_CODE_F7;
        case GERIUM_SCANCODE_F8:
            return GERIUM_KEY_CODE_F8;
        case GERIUM_SCANCODE_F9:
            return GERIUM_KEY_CODE_F9;
        case GERIUM_SCANCODE_F10:
            return GERIUM_KEY_CODE_F10;
        case GERIUM_SCANCODE_F11:
            return GERIUM_KEY_CODE_F11;
        case GERIUM_SCANCODE_F12:
            return GERIUM_KEY_CODE_F12;
        case GERIUM_SCANCODE_F13:
            return GERIUM_KEY_CODE_F13;
        case GERIUM_SCANCODE_F14:
            return GERIUM_KEY_CODE_F14;
        case GERIUM_SCANCODE_F15:
            return GERIUM_KEY_CODE_F15;
        case GERIUM_SCANCODE_F16:
            return GERIUM_KEY_CODE_F16;
        case GERIUM_SCANCODE_F17:
            return GERIUM_KEY_CODE_F17;
        case GERIUM_SCANCODE_F18:
            return GERIUM_KEY_CODE_F18;
        case GERIUM_SCANCODE_F19:
            return GERIUM_KEY_CODE_F19;
        case GERIUM_SCANCODE_F20:
            return GERIUM_KEY_CODE_F20;
        case GERIUM_SCANCODE_F21:
            return GERIUM_KEY_CODE_F21;
        case GERIUM_SCANCODE_F22:
            return GERIUM_KEY_CODE_F22;
        case GERIUM_SCANCODE_F23:
            return GERIUM_KEY_CODE_F23;
        case GERIUM_SCANCODE_F24:
            return GERIUM_KEY_CODE_F24;
        case GERIUM_SCANCODE_NUMPAD_0:
            return numlock ? GERIUM_KEY_CODE_0 : GERIUM_KEY_CODE_INSERT;
        case GERIUM_SCANCODE_NUMPAD_1:
            return numlock ? GERIUM_KEY_CODE_1 : GERIUM_KEY_CODE_END;
        case GERIUM_SCANCODE_NUMPAD_2:
            return numlock ? GERIUM_KEY_CODE_2 : GERIUM_KEY_CODE_ARROW_DOWN;
        case GERIUM_SCANCODE_NUMPAD_3:
            return numlock ? GERIUM_KEY_CODE_3 : GERIUM_KEY_CODE_PAGE_DOWN;
        case GERIUM_SCANCODE_NUMPAD_4:
            return numlock ? GERIUM_KEY_CODE_4 : GERIUM_KEY_CODE_ARROW_LEFT;
        case GERIUM_SCANCODE_NUMPAD_5:
            return numlock ? GERIUM_KEY_CODE_5 : GERIUM_KEY_CODE_UNKNOWN;
        case GERIUM_SCANCODE_NUMPAD_6:
            return numlock ? GERIUM_KEY_CODE_6 : GERIUM_KEY_CODE_ARROW_RIGHT;
        case GERIUM_SCANCODE_NUMPAD_7:
            return numlock ? GERIUM_KEY_CODE_7 : GERIUM_KEY_CODE_HOME;
        case GERIUM_SCANCODE_NUMPAD_8:
            return numlock ? GERIUM_KEY_CODE_8 : GERIUM_KEY_CODE_ARROW_UP;
        case GERIUM_SCANCODE_NUMPAD_9:
            return numlock ? GERIUM_KEY_CODE_9 : GERIUM_KEY_CODE_PAGE_UP;
        case GERIUM_SCANCODE_NUMPAD_COMMA:
            return GERIUM_KEY_CODE_COMMA;
        case GERIUM_SCANCODE_NUMPAD_ENTER:
            return GERIUM_KEY_CODE_ENTER;
        case GERIUM_SCANCODE_NUMPAD_EQUAL:
            return GERIUM_KEY_CODE_EQUAL;
        case GERIUM_SCANCODE_NUMPAD_SUBTRACT:
            return GERIUM_KEY_CODE_SUBTRACT;
        case GERIUM_SCANCODE_NUMPAD_DECIMAL:
            return numlock ? GERIUM_KEY_CODE_PERIOD : GERIUM_KEY_CODE_DELETE;
        case GERIUM_SCANCODE_NUMPAD_ADD:
            return GERIUM_KEY_CODE_ADD;
        case GERIUM_SCANCODE_NUMPAD_DIVIDE:
            return GERIUM_KEY_CODE_DIVIDE;
        case GERIUM_SCANCODE_NUMPAD_MULTIPLY:
            return GERIUM_KEY_CODE_MULTIPLY;
        case GERIUM_SCANCODE_NUM_LOCK:
            return GERIUM_KEY_CODE_NUM_LOCK;
        case GERIUM_SCANCODE_MEDIA_PLAY_PAUSE:
            return GERIUM_KEY_CODE_MEDIA_PLAY_PAUSE;
        case GERIUM_SCANCODE_MEDIA_TRACK_PREVIOUS:
            return GERIUM_KEY_CODE_MEDIA_TRACK_PREVIOUS;
        case GERIUM_SCANCODE_MEDIA_TRACK_NEXT:
            return GERIUM_KEY_CODE_MEDIA_TRACK_NEXT;
        case GERIUM_SCANCODE_MEDIA_STOP:
            return GERIUM_KEY_CODE_MEDIA_STOP;
        case GERIUM_SCANCODE_LAUNCH_MEDIA_PLAYER:
            return GERIUM_KEY_CODE_LAUNCH_MEDIA_PLAYER;
        case GERIUM_SCANCODE_AUDIO_VOLUME_MUTE:
            return GERIUM_KEY_CODE_AUDIO_VOLUME_MUTE;
        case GERIUM_SCANCODE_AUDIO_VOLUME_DOWN:
            return GERIUM_KEY_CODE_AUDIO_VOLUME_DOWN;
        case GERIUM_SCANCODE_AUDIO_VOLUME_UP:
            return GERIUM_KEY_CODE_AUDIO_VOLUME_UP;
        case GERIUM_SCANCODE_ESCAPE:
            return GERIUM_KEY_CODE_ESCAPE;
        case GERIUM_SCANCODE_TAB:
            return GERIUM_KEY_CODE_TAB;
        case GERIUM_SCANCODE_CAPS_LOCK:
            return GERIUM_KEY_CODE_CAPS_LOCK;
        case GERIUM_SCANCODE_ENTER:
            return GERIUM_KEY_CODE_ENTER;
        case GERIUM_SCANCODE_BACKSLASH:
            return shift ? GERIUM_KEY_CODE_PIPE : GERIUM_KEY_CODE_BACKSLASH;
        case GERIUM_SCANCODE_BACKSPACE:
            return GERIUM_KEY_CODE_BACKSPACE;
        case GERIUM_SCANCODE_INTL_BACKSLASH:
            return GERIUM_KEY_CODE_UNKNOWN;
        case GERIUM_SCANCODE_INTL_RO:
            return GERIUM_KEY_CODE_UNKNOWN;
        case GERIUM_SCANCODE_INTL_YEN:
            return GERIUM_KEY_CODE_UNKNOWN;
        case GERIUM_SCANCODE_MINUS:
            return shift ? GERIUM_KEY_CODE_UNDERSCORE : GERIUM_KEY_CODE_SUBTRACT;
        case GERIUM_SCANCODE_COLON:
            return GERIUM_KEY_CODE_COLON;
        case GERIUM_SCANCODE_COMMA:
            return shift ? GERIUM_KEY_CODE_LESS : GERIUM_KEY_CODE_COMMA;
        case GERIUM_SCANCODE_CONVERT:
            return GERIUM_KEY_CODE_CONVERT;
        case GERIUM_SCANCODE_NONCONVERT:
            return GERIUM_KEY_CODE_NONCONVERT;
        case GERIUM_SCANCODE_EQUAL:
            return shift ? GERIUM_KEY_CODE_EQUAL : GERIUM_KEY_CODE_EQUAL;
        case GERIUM_SCANCODE_PERIOD:
            return shift ? GERIUM_KEY_CODE_GREATER : GERIUM_KEY_CODE_PERIOD;
        case GERIUM_SCANCODE_POWER:
            return GERIUM_KEY_CODE_POWER;
        case GERIUM_SCANCODE_SEMICOLON:
            return shift ? GERIUM_KEY_CODE_COLON : GERIUM_KEY_CODE_SEMICOLON;
        case GERIUM_SCANCODE_SLASH:
            return shift ? GERIUM_KEY_CODE_QUESTION : GERIUM_KEY_CODE_SLASH;
        case GERIUM_SCANCODE_SLEEP:
            return GERIUM_KEY_CODE_SLEEP;
        case GERIUM_SCANCODE_WAKE:
            return GERIUM_KEY_CODE_WAKE;
        case GERIUM_SCANCODE_SPACE:
            return GERIUM_KEY_CODE_SPACE;
        case GERIUM_SCANCODE_QUOTE:
            return shift ? GERIUM_KEY_CODE_DOUBLE_QUOTE : GERIUM_KEY_CODE_QUOTE;
        case GERIUM_SCANCODE_BACKQUOTE:
            return shift ? GERIUM_KEY_CODE_TILDE : GERIUM_KEY_CODE_BACKQUOTE;
        case GERIUM_SCANCODE_ALT_LEFT:
            return GERIUM_KEY_CODE_ALT_LEFT;
        case GERIUM_SCANCODE_ALT_RIGHT:
            return GERIUM_KEY_CODE_ALT_RIGHT;
        case GERIUM_SCANCODE_BRACKET_LEFT:
            return shift ? GERIUM_KEY_CODE_BRACE_LEFT : GERIUM_KEY_CODE_BRACKET_LEFT;
        case GERIUM_SCANCODE_BRACKET_RIGHT:
            return shift ? GERIUM_KEY_CODE_BRACE_RIGHT : GERIUM_KEY_CODE_BRACKET_RIGHT;
        case GERIUM_SCANCODE_CONTROL_LEFT:
            return GERIUM_KEY_CODE_CONTROL_LEFT;
        case GERIUM_SCANCODE_CONTROL_RIGHT:
            return GERIUM_KEY_CODE_CONTROL_RIGHT;
        case GERIUM_SCANCODE_SHIFT_LEFT:
            return GERIUM_KEY_CODE_SHIFT_LEFT;
        case GERIUM_SCANCODE_SHIFT_RIGHT:
            return GERIUM_KEY_CODE_SHIFT_RIGHT;
        case GERIUM_SCANCODE_META_LEFT:
            return GERIUM_KEY_CODE_META_LEFT;
        case GERIUM_SCANCODE_META_RIGHT:
            return GERIUM_KEY_CODE_META_RIGHT;
        case GERIUM_SCANCODE_ARROW_UP:
            return GERIUM_KEY_CODE_ARROW_UP;
        case GERIUM_SCANCODE_ARROW_LEFT:
            return GERIUM_KEY_CODE_ARROW_LEFT;
        case GERIUM_SCANCODE_ARROW_RIGHT:
            return GERIUM_KEY_CODE_ARROW_RIGHT;
        case GERIUM_SCANCODE_ARROW_DOWN:
            return GERIUM_KEY_CODE_ARROW_DOWN;
        case GERIUM_SCANCODE_SCROLL_LOCK:
            return GERIUM_KEY_CODE_SCROLL_LOCK;
        case GERIUM_SCANCODE_PAUSE:
            return GERIUM_KEY_CODE_PAUSE;
        case GERIUM_SCANCODE_CTRL_PAUSE:
            return GERIUM_KEY_CODE_UNKNOWN;
        case GERIUM_SCANCODE_INSERT:
            return GERIUM_KEY_CODE_INSERT;
        case GERIUM_SCANCODE_DELETE:
            return GERIUM_KEY_CODE_DELETE;
        case GERIUM_SCANCODE_HOME:
            return GERIUM_KEY_CODE_HOME;
        case GERIUM_SCANCODE_END:
            return GERIUM_KEY_CODE_END;
        case GERIUM_SCANCODE_PAGE_UP:
            return GERIUM_KEY_CODE_PAGE_UP;
        case GERIUM_SCANCODE_PAGE_DOWN:
            return GERIUM_KEY_CODE_PAGE_DOWN;
        case GERIUM_SCANCODE_LAUNCH_MAIL:
            return GERIUM_KEY_CODE_LAUNCH_MAIL;
        case GERIUM_SCANCODE_MYCOMPUTER:
            return GERIUM_KEY_CODE_UNKNOWN;
        case GERIUM_SCANCODE_CONTEXT_MENU:
            return GERIUM_KEY_CODE_CONTEXT_MENU;
        case GERIUM_SCANCODE_PRINT_SCREEN:
            return GERIUM_KEY_CODE_PRINT_SCREEN;
        case GERIUM_SCANCODE_ALT_PRINT_SCREEN:
            return GERIUM_KEY_CODE_UNKNOWN;
        case GERIUM_SCANCODE_LAUNCH_APPLICATION_1:
            return GERIUM_KEY_CODE_LAUNCH_APPLICATION_1;
        case GERIUM_SCANCODE_LAUNCH_APPLICATION_2:
            return GERIUM_KEY_CODE_LAUNCH_APPLICATION_2;
        case GERIUM_SCANCODE_KANA_MODE:
            return GERIUM_KEY_CODE_KANA_MODE;
        case GERIUM_SCANCODE_BROWSER_BACK:
            return GERIUM_KEY_CODE_BROWSER_BACK;
        case GERIUM_SCANCODE_BROWSER_FAVORITES:
            return GERIUM_KEY_CODE_BROWSER_FAVORITES;
        case GERIUM_SCANCODE_BROWSER_FORWARD:
            return GERIUM_KEY_CODE_BROWSER_FORWARD;
        case GERIUM_SCANCODE_BROWSER_HOME:
            return GERIUM_KEY_CODE_BROWSER_HOME;
        case GERIUM_SCANCODE_BROWSER_REFRESH:
            return GERIUM_KEY_CODE_BROWSER_REFRESH;
        case GERIUM_SCANCODE_BROWSER_SEARCH:
            return GERIUM_KEY_CODE_BROWSER_SEARCH;
        case GERIUM_SCANCODE_BROWSER_STOP:
            return GERIUM_KEY_CODE_BROWSER_STOP;
        default:
            return GERIUM_KEY_CODE_UNKNOWN;
    }
}

gerium_inline ImGuiKey toImguiKey(gerium_scancode_t scanCode) noexcept {
    switch (scanCode) {
        case GERIUM_SCANCODE_UNKNOWN:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_0:
            return ImGuiKey_0;
        case GERIUM_SCANCODE_1:
            return ImGuiKey_1;
        case GERIUM_SCANCODE_2:
            return ImGuiKey_2;
        case GERIUM_SCANCODE_3:
            return ImGuiKey_3;
        case GERIUM_SCANCODE_4:
            return ImGuiKey_4;
        case GERIUM_SCANCODE_5:
            return ImGuiKey_5;
        case GERIUM_SCANCODE_6:
            return ImGuiKey_6;
        case GERIUM_SCANCODE_7:
            return ImGuiKey_7;
        case GERIUM_SCANCODE_8:
            return ImGuiKey_8;
        case GERIUM_SCANCODE_9:
            return ImGuiKey_9;
        case GERIUM_SCANCODE_A:
            return ImGuiKey_A;
        case GERIUM_SCANCODE_B:
            return ImGuiKey_B;
        case GERIUM_SCANCODE_C:
            return ImGuiKey_C;
        case GERIUM_SCANCODE_D:
            return ImGuiKey_D;
        case GERIUM_SCANCODE_E:
            return ImGuiKey_E;
        case GERIUM_SCANCODE_F:
            return ImGuiKey_F;
        case GERIUM_SCANCODE_G:
            return ImGuiKey_G;
        case GERIUM_SCANCODE_H:
            return ImGuiKey_H;
        case GERIUM_SCANCODE_I:
            return ImGuiKey_I;
        case GERIUM_SCANCODE_J:
            return ImGuiKey_J;
        case GERIUM_SCANCODE_K:
            return ImGuiKey_K;
        case GERIUM_SCANCODE_L:
            return ImGuiKey_L;
        case GERIUM_SCANCODE_M:
            return ImGuiKey_M;
        case GERIUM_SCANCODE_N:
            return ImGuiKey_N;
        case GERIUM_SCANCODE_O:
            return ImGuiKey_O;
        case GERIUM_SCANCODE_P:
            return ImGuiKey_P;
        case GERIUM_SCANCODE_Q:
            return ImGuiKey_Q;
        case GERIUM_SCANCODE_R:
            return ImGuiKey_R;
        case GERIUM_SCANCODE_S:
            return ImGuiKey_S;
        case GERIUM_SCANCODE_T:
            return ImGuiKey_T;
        case GERIUM_SCANCODE_U:
            return ImGuiKey_U;
        case GERIUM_SCANCODE_V:
            return ImGuiKey_V;
        case GERIUM_SCANCODE_W:
            return ImGuiKey_W;
        case GERIUM_SCANCODE_X:
            return ImGuiKey_X;
        case GERIUM_SCANCODE_Y:
            return ImGuiKey_Y;
        case GERIUM_SCANCODE_Z:
            return ImGuiKey_Z;
        case GERIUM_SCANCODE_F1:
            return ImGuiKey_F1;
        case GERIUM_SCANCODE_F2:
            return ImGuiKey_F2;
        case GERIUM_SCANCODE_F3:
            return ImGuiKey_F3;
        case GERIUM_SCANCODE_F4:
            return ImGuiKey_F4;
        case GERIUM_SCANCODE_F5:
            return ImGuiKey_F5;
        case GERIUM_SCANCODE_F6:
            return ImGuiKey_F6;
        case GERIUM_SCANCODE_F7:
            return ImGuiKey_F7;
        case GERIUM_SCANCODE_F8:
            return ImGuiKey_F8;
        case GERIUM_SCANCODE_F9:
            return ImGuiKey_F9;
        case GERIUM_SCANCODE_F10:
            return ImGuiKey_F10;
        case GERIUM_SCANCODE_F11:
            return ImGuiKey_F11;
        case GERIUM_SCANCODE_F12:
            return ImGuiKey_F12;
        case GERIUM_SCANCODE_F13:
            return ImGuiKey_F13;
        case GERIUM_SCANCODE_F14:
            return ImGuiKey_F14;
        case GERIUM_SCANCODE_F15:
            return ImGuiKey_F15;
        case GERIUM_SCANCODE_F16:
            return ImGuiKey_F16;
        case GERIUM_SCANCODE_F17:
            return ImGuiKey_F17;
        case GERIUM_SCANCODE_F18:
            return ImGuiKey_F18;
        case GERIUM_SCANCODE_F19:
            return ImGuiKey_F19;
        case GERIUM_SCANCODE_F20:
            return ImGuiKey_F20;
        case GERIUM_SCANCODE_F21:
            return ImGuiKey_F21;
        case GERIUM_SCANCODE_F22:
            return ImGuiKey_F22;
        case GERIUM_SCANCODE_F23:
            return ImGuiKey_F23;
        case GERIUM_SCANCODE_F24:
            return ImGuiKey_F24;
        case GERIUM_SCANCODE_NUMPAD_0:
            return ImGuiKey_Keypad0;
        case GERIUM_SCANCODE_NUMPAD_1:
            return ImGuiKey_Keypad1;
        case GERIUM_SCANCODE_NUMPAD_2:
            return ImGuiKey_Keypad2;
        case GERIUM_SCANCODE_NUMPAD_3:
            return ImGuiKey_Keypad3;
        case GERIUM_SCANCODE_NUMPAD_4:
            return ImGuiKey_Keypad4;
        case GERIUM_SCANCODE_NUMPAD_5:
            return ImGuiKey_Keypad5;
        case GERIUM_SCANCODE_NUMPAD_6:
            return ImGuiKey_Keypad6;
        case GERIUM_SCANCODE_NUMPAD_7:
            return ImGuiKey_Keypad7;
        case GERIUM_SCANCODE_NUMPAD_8:
            return ImGuiKey_Keypad8;
        case GERIUM_SCANCODE_NUMPAD_9:
            return ImGuiKey_Keypad9;
        case GERIUM_SCANCODE_NUMPAD_COMMA:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_NUMPAD_ENTER:
            return ImGuiKey_KeypadEnter;
        case GERIUM_SCANCODE_NUMPAD_EQUAL:
            return ImGuiKey_KeypadEqual;
        case GERIUM_SCANCODE_NUMPAD_SUBTRACT:
            return ImGuiKey_KeypadSubtract;
        case GERIUM_SCANCODE_NUMPAD_DECIMAL:
            return ImGuiKey_KeypadDecimal;
        case GERIUM_SCANCODE_NUMPAD_ADD:
            return ImGuiKey_KeypadAdd;
        case GERIUM_SCANCODE_NUMPAD_DIVIDE:
            return ImGuiKey_KeypadDivide;
        case GERIUM_SCANCODE_NUMPAD_MULTIPLY:
            return ImGuiKey_KeypadMultiply;
        case GERIUM_SCANCODE_NUM_LOCK:
            return ImGuiKey_NumLock;
        case GERIUM_SCANCODE_MEDIA_PLAY_PAUSE:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_MEDIA_TRACK_PREVIOUS:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_MEDIA_TRACK_NEXT:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_MEDIA_STOP:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_LAUNCH_MEDIA_PLAYER:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_AUDIO_VOLUME_MUTE:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_AUDIO_VOLUME_DOWN:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_AUDIO_VOLUME_UP:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_ESCAPE:
            return ImGuiKey_Escape;
        case GERIUM_SCANCODE_TAB:
            return ImGuiKey_Tab;
        case GERIUM_SCANCODE_CAPS_LOCK:
            return ImGuiKey_CapsLock;
        case GERIUM_SCANCODE_ENTER:
            return ImGuiKey_Enter;
        case GERIUM_SCANCODE_BACKSLASH:
            return ImGuiKey_Backslash;
        case GERIUM_SCANCODE_BACKSPACE:
            return ImGuiKey_Backspace;
        case GERIUM_SCANCODE_INTL_BACKSLASH:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_INTL_RO:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_INTL_YEN:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_MINUS:
            return ImGuiKey_Minus;
        case GERIUM_SCANCODE_COLON:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_COMMA:
            return ImGuiKey_Comma;
        case GERIUM_SCANCODE_CONVERT:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_NONCONVERT:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_EQUAL:
            return ImGuiKey_Equal;
        case GERIUM_SCANCODE_PERIOD:
            return ImGuiKey_Period;
        case GERIUM_SCANCODE_POWER:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_SEMICOLON:
            return ImGuiKey_Semicolon;
        case GERIUM_SCANCODE_SLASH:
            return ImGuiKey_Slash;
        case GERIUM_SCANCODE_SLEEP:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_WAKE:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_SPACE:
            return ImGuiKey_Space;
        case GERIUM_SCANCODE_QUOTE:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_BACKQUOTE:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_ALT_LEFT:
            return ImGuiKey_LeftAlt;
        case GERIUM_SCANCODE_ALT_RIGHT:
            return ImGuiKey_RightAlt;
        case GERIUM_SCANCODE_BRACKET_LEFT:
            return ImGuiKey_LeftBracket;
        case GERIUM_SCANCODE_BRACKET_RIGHT:
            return ImGuiKey_RightBracket;
        case GERIUM_SCANCODE_CONTROL_LEFT:
            return ImGuiKey_LeftCtrl;
        case GERIUM_SCANCODE_CONTROL_RIGHT:
            return ImGuiKey_RightCtrl;
        case GERIUM_SCANCODE_SHIFT_LEFT:
            return ImGuiKey_LeftShift;
        case GERIUM_SCANCODE_SHIFT_RIGHT:
            return ImGuiKey_RightShift;
        case GERIUM_SCANCODE_META_LEFT:
            return ImGuiKey_LeftSuper;
        case GERIUM_SCANCODE_META_RIGHT:
            return ImGuiKey_RightSuper;
        case GERIUM_SCANCODE_ARROW_UP:
            return ImGuiKey_UpArrow;
        case GERIUM_SCANCODE_ARROW_LEFT:
            return ImGuiKey_LeftArrow;
        case GERIUM_SCANCODE_ARROW_RIGHT:
            return ImGuiKey_RightArrow;
        case GERIUM_SCANCODE_ARROW_DOWN:
            return ImGuiKey_DownArrow;
        case GERIUM_SCANCODE_SCROLL_LOCK:
            return ImGuiKey_ScrollLock;
        case GERIUM_SCANCODE_PAUSE:
            return ImGuiKey_Pause;
        case GERIUM_SCANCODE_CTRL_PAUSE:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_INSERT:
            return ImGuiKey_Insert;
        case GERIUM_SCANCODE_DELETE:
            return ImGuiKey_Delete;
        case GERIUM_SCANCODE_HOME:
            return ImGuiKey_Home;
        case GERIUM_SCANCODE_END:
            return ImGuiKey_End;
        case GERIUM_SCANCODE_PAGE_UP:
            return ImGuiKey_PageUp;
        case GERIUM_SCANCODE_PAGE_DOWN:
            return ImGuiKey_PageDown;
        case GERIUM_SCANCODE_LAUNCH_MAIL:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_MYCOMPUTER:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_CONTEXT_MENU:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_PRINT_SCREEN:
            return ImGuiKey_PrintScreen;
        case GERIUM_SCANCODE_ALT_PRINT_SCREEN:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_LAUNCH_APPLICATION_1:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_LAUNCH_APPLICATION_2:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_KANA_MODE:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_BROWSER_BACK:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_BROWSER_FAVORITES:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_BROWSER_FORWARD:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_BROWSER_HOME:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_BROWSER_REFRESH:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_BROWSER_SEARCH:
            return ImGuiKey_None;
        case GERIUM_SCANCODE_BROWSER_STOP:
            return ImGuiKey_None;
        default:
            return ImGuiKey_None;
    }
}

} // namespace gerium::linux

#endif
