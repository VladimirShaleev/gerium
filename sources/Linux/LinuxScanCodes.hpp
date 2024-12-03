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

} // namespace gerium::linux

#endif
