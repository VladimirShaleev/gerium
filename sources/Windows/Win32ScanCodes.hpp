#ifndef GERIUM_WINDOWS_WIN32_SCAN_CODES_HPP
#define GERIUM_WINDOWS_WIN32_SCAN_CODES_HPP

#include "../Gerium.hpp"

namespace gerium::windows {

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
    Pause              = 0x0045,
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
    AltPrintScreen     = 0x0054,
    IntlBackslash      = 0x0056,
    F11                = 0x0057,
    F12                = 0x0058,
    NumpadEqual        = 0x0059,
    F13                = 0x0064,
    F14                = 0x0065,
    F15                = 0x0066,
    F16                = 0x0067,
    F17                = 0x0068,
    F18                = 0x0069,
    F19                = 0x006A,
    F20                = 0x006B,
    F21                = 0x006C,
    F22                = 0x006D,
    F23                = 0x006E,
    KanaMode           = 0x0070,
    Lang2              = 0x0071,
    Lang1              = 0x0072,
    IntlRo             = 0x0073,
    F24                = 0x0076,
    Convert            = 0x0079,
    NonConvert         = 0x007B,
    IntlYen            = 0x007D,
    NumpadComma        = 0x007E,
    Paste              = 0xE00A,
    MediaTrackPrevious = 0xE010,
    MediaTrackNext     = 0xE019,
    NumpadEnter        = 0xE01C,
    ControlRight       = 0xE01D,
    AudioVolumeMute    = 0xE020,
    LaunchApp2         = 0xE021,
    MediaPlayPause     = 0xE022,
    MediaStop          = 0xE024,
    AudioVolumeDown    = 0xE02E,
    AudioVolumeUp      = 0xE030,
    BrowserHome        = 0xE032,
    NumpadDivide       = 0xE035,
    PrintScreen        = 0xE037,
    AltRight           = 0xE038,
    NumLock            = 0xE045,
    CtrlPause          = 0xE046,
    Home               = 0xE047,
    ArrowUp            = 0xE048,
    PageUp             = 0xE049,
    ArrowLeft          = 0xE04B,
    ArrowRight         = 0xE04D,
    End                = 0xE04F,
    ArrowDown          = 0xE050,
    PageDown           = 0xE051,
    Insert             = 0xE052,
    Delete             = 0xE053,
    MetaLeft           = 0xE05B,
    MetaRight          = 0xE05C,
    ContextMenu        = 0xE05D,
    Power              = 0xE05E,
    Sleep              = 0xE05F,
    Wake               = 0xE063,
    BrowserSearch      = 0xE065,
    BrowserFavorites   = 0xE066,
    BrowserRefresh     = 0xE067,
    BrowserStop        = 0xE068,
    BrowserForward     = 0xE069,
    BrowserBack        = 0xE06A,
    LaunchApp1         = 0xE06B,
    LaunchMail         = 0xE06C,
    LaunchMediaPlayer  = 0xE06D,
    Lang2KL            = 0xE0F1,
    Lang1KL            = 0xE0F2
};

gerium_inline gerium_scancode_t toScanCode(ScanCode scanCode) noexcept {
    switch (scanCode) {
        case ScanCode::Escape:
            return GERIUM_SCANCODE_ESCAPE;
        case ScanCode::Digit0:
            return GERIUM_SCANCODE_0;
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
        case ScanCode::Pause:
            return GERIUM_SCANCODE_PAUSE;
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
        case ScanCode::AltPrintScreen:
            return GERIUM_SCANCODE_ALT_PRINT_SCREEN;
        case ScanCode::IntlBackslash:
            return GERIUM_SCANCODE_INTL_BACKSLASH;
        case ScanCode::F11:
            return GERIUM_SCANCODE_F11;
        case ScanCode::F12:
            return GERIUM_SCANCODE_F12;
        case ScanCode::NumpadEqual:
            return GERIUM_SCANCODE_NUMPAD_EQUAL;
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
        case ScanCode::KanaMode:
            return GERIUM_SCANCODE_KANA_MODE;
        case ScanCode::IntlRo:
            return GERIUM_SCANCODE_INTL_RO;
        case ScanCode::F24:
            return GERIUM_SCANCODE_F24;
        case ScanCode::Convert:
            return GERIUM_SCANCODE_CONVERT;
        case ScanCode::NonConvert:
            return GERIUM_SCANCODE_NONCONVERT;
        case ScanCode::IntlYen:
            return GERIUM_SCANCODE_INTL_YEN;
        case ScanCode::NumpadComma:
            return GERIUM_SCANCODE_NUMPAD_COMMA;
        case ScanCode::MediaTrackPrevious:
            return GERIUM_SCANCODE_MEDIA_TRACK_PREVIOUS;
        case ScanCode::MediaTrackNext:
            return GERIUM_SCANCODE_MEDIA_TRACK_NEXT;
        case ScanCode::NumpadEnter:
            return GERIUM_SCANCODE_NUMPAD_ENTER;
        case ScanCode::ControlRight:
            return GERIUM_SCANCODE_CONTROL_RIGHT;
        case ScanCode::AudioVolumeMute:
            return GERIUM_SCANCODE_AUDIO_VOLUME_MUTE;
        case ScanCode::LaunchApp2:
            return GERIUM_SCANCODE_LAUNCH_APPLICATION_2;
        case ScanCode::MediaPlayPause:
            return GERIUM_SCANCODE_MEDIA_PLAY_PAUSE;
        case ScanCode::MediaStop:
            return GERIUM_SCANCODE_MEDIA_STOP;
        case ScanCode::AudioVolumeDown:
            return GERIUM_SCANCODE_AUDIO_VOLUME_DOWN;
        case ScanCode::AudioVolumeUp:
            return GERIUM_SCANCODE_AUDIO_VOLUME_UP;
        case ScanCode::BrowserHome:
            return GERIUM_SCANCODE_BROWSER_HOME;
        case ScanCode::NumpadDivide:
            return GERIUM_SCANCODE_NUMPAD_DIVIDE;
        case ScanCode::PrintScreen:
            return GERIUM_SCANCODE_PRINT_SCREEN;
        case ScanCode::AltRight:
            return GERIUM_SCANCODE_ALT_RIGHT;
        case ScanCode::NumLock:
            return GERIUM_SCANCODE_NUM_LOCK;
        case ScanCode::CtrlPause:
            return GERIUM_SCANCODE_CTRL_PAUSE;
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
        case ScanCode::MetaLeft:
            return GERIUM_SCANCODE_META_LEFT;
        case ScanCode::MetaRight:
            return GERIUM_SCANCODE_META_RIGHT;
        case ScanCode::ContextMenu:
            return GERIUM_SCANCODE_CONTEXT_MENU;
        case ScanCode::Power:
            return GERIUM_SCANCODE_POWER;
        case ScanCode::Sleep:
            return GERIUM_SCANCODE_SLEEP;
        case ScanCode::Wake:
            return GERIUM_SCANCODE_WAKE;
        case ScanCode::BrowserSearch:
            return GERIUM_SCANCODE_BROWSER_SEARCH;
        case ScanCode::BrowserFavorites:
            return GERIUM_SCANCODE_BROWSER_FAVORITES;
        case ScanCode::BrowserRefresh:
            return GERIUM_SCANCODE_BROWSER_REFRESH;
        case ScanCode::BrowserStop:
            return GERIUM_SCANCODE_BROWSER_STOP;
        case ScanCode::BrowserForward:
            return GERIUM_SCANCODE_BROWSER_FORWARD;
        case ScanCode::BrowserBack:
            return GERIUM_SCANCODE_BROWSER_BACK;
        case ScanCode::LaunchApp1:
            return GERIUM_SCANCODE_LAUNCH_APPLICATION_1;
        case ScanCode::LaunchMail:
            return GERIUM_SCANCODE_LAUNCH_MAIL;
        case ScanCode::LaunchMediaPlayer:
            return GERIUM_SCANCODE_LAUNCH_MEDIA_PLAYER;
        case ScanCode::Paste:
        case ScanCode::Lang2:
        case ScanCode::Lang1:
        case ScanCode::Lang2KL:
        case ScanCode::Lang1KL:
        default:
            return GERIUM_SCANCODE_UNKNOWN;
    }
}

gerium_inline gerium_key_code_t toKeyCode(USHORT vkey, gerium_scancode_t scancode, bool shift) {
    if (vkey >= 0x30 && vkey <= 0x39) {
        auto num = int(shift ? GERIUM_KEY_CODE_EXCLAIM : GERIUM_KEY_CODE_0);
        return gerium_key_code_t(num + (vkey - 0x30));
    }

    if (vkey >= 0x41 && vkey <= 0x5A) {
        return gerium_key_code_t(int(GERIUM_KEY_CODE_EXCLAIM) + (vkey - 0x41));
    }

    if (vkey >= VK_NUMPAD0 && vkey <= VK_NUMPAD9) {
        return gerium_key_code_t(int(GERIUM_KEY_CODE_NUMPAD_0) + (vkey - VK_NUMPAD0));
    }

    if (vkey >= VK_F1 && vkey <= VK_F24) {
        return gerium_key_code_t(int(GERIUM_KEY_CODE_F1) + (vkey - VK_F1));
    }

    switch (vkey) {
        case VK_LBUTTON:
        case VK_RBUTTON:
        case VK_CANCEL:
        case VK_MBUTTON:
        case VK_XBUTTON1:
        case VK_XBUTTON2:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_BACK:
            return GERIUM_KEY_CODE_BACKSPACE;
        case VK_TAB:
            return GERIUM_KEY_CODE_TAB;
        case VK_CLEAR:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_RETURN:
            return GERIUM_KEY_CODE_ENTER;
        case VK_SHIFT:
            return scancode == GERIUM_SCANCODE_SHIFT_LEFT ? GERIUM_KEY_CODE_SHIFT_LEFT : GERIUM_KEY_CODE_SHIFT_RIGHT;
        case VK_CONTROL:
            return scancode == GERIUM_SCANCODE_CONTROL_LEFT ? GERIUM_KEY_CODE_CONTROL_LEFT
                                                            : GERIUM_KEY_CODE_CONTROL_RIGHT;
        case VK_MENU:
            return scancode == GERIUM_SCANCODE_ALT_LEFT ? GERIUM_KEY_CODE_ALT_LEFT : GERIUM_KEY_CODE_ALT_RIGHT;
        case VK_PAUSE:
            return GERIUM_KEY_CODE_PAUSE;
        case VK_CAPITAL:
            return GERIUM_KEY_CODE_CAPS_LOCK;
        case VK_KANA:
            return GERIUM_KEY_CODE_KANA_MODE;
        case VK_IME_ON:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_JUNJA:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_FINAL:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_KANJI:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_IME_OFF:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_ESCAPE:
            return GERIUM_KEY_CODE_ESCAPE;
        case VK_CONVERT:
            return GERIUM_KEY_CODE_CONVERT;
        case VK_NONCONVERT:
            return GERIUM_KEY_CODE_NONCONVERT;
        case VK_ACCEPT:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_MODECHANGE:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_SPACE:
            return GERIUM_KEY_CODE_SPACE;
        case VK_PRIOR:
            return GERIUM_KEY_CODE_PAGE_UP;
        case VK_NEXT:
            return GERIUM_KEY_CODE_PAGE_DOWN;
        case VK_END:
            return GERIUM_KEY_CODE_END;
        case VK_HOME:
            return GERIUM_KEY_CODE_HOME;
        case VK_LEFT:
            return GERIUM_KEY_CODE_ARROW_LEFT;
        case VK_UP:
            return GERIUM_KEY_CODE_ARROW_UP;
        case VK_RIGHT:
            return GERIUM_KEY_CODE_ARROW_RIGHT;
        case VK_DOWN:
            return GERIUM_KEY_CODE_ARROW_DOWN;
        case VK_SELECT:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_PRINT:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_EXECUTE:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_SNAPSHOT:
            return GERIUM_KEY_CODE_PRINT_SCREEN;
        case VK_INSERT:
            return GERIUM_KEY_CODE_INSERT;
        case VK_DELETE:
            return GERIUM_KEY_CODE_DELETE;
        case VK_HELP:
            return GERIUM_KEY_CODE_UNKNOWN;
        case VK_LWIN:
            return GERIUM_KEY_CODE_META_LEFT;
        case VK_RWIN:
            return GERIUM_KEY_CODE_META_RIGHT;
        case VK_APPS:
            return GERIUM_KEY_CODE_CONTEXT_MENU;
        case VK_SLEEP:
            return GERIUM_KEY_CODE_SLEEP;
        case VK_MULTIPLY:
            return GERIUM_KEY_CODE_MULTIPLY;
        case VK_ADD:
            return shift ? GERIUM_KEY_CODE_ADD : GERIUM_KEY_CODE_EQUAL;
        case VK_SEPARATOR:
            return GERIUM_KEY_CODE_PERIOD;
        case VK_SUBTRACT:
            return GERIUM_KEY_CODE_SUBTRACT;
        case VK_DECIMAL:
            return GERIUM_KEY_CODE_DECIMAL;
        case VK_DIVIDE:
            return GERIUM_KEY_CODE_DIVIDE;
        case VK_NUMLOCK:
            return GERIUM_KEY_CODE_NUM_LOCK;
        case VK_SCROLL:
            return GERIUM_KEY_CODE_SCROLL_LOCK;
        case VK_LSHIFT:
            return GERIUM_KEY_CODE_SHIFT_LEFT;
        case VK_RSHIFT:
            return GERIUM_KEY_CODE_SHIFT_RIGHT;
        case VK_LCONTROL:
            return GERIUM_KEY_CODE_CONTROL_LEFT;
        case VK_RCONTROL:
            return GERIUM_KEY_CODE_CONTROL_RIGHT;
        case VK_LMENU:
            return GERIUM_KEY_CODE_ALT_LEFT;
        case VK_RMENU:
            return GERIUM_KEY_CODE_ALT_RIGHT;
        case VK_BROWSER_BACK:
            return GERIUM_KEY_CODE_BROWSER_BACK;
        case VK_BROWSER_FORWARD:
            return GERIUM_KEY_CODE_BROWSER_FORWARD;
        case VK_BROWSER_REFRESH:
            return GERIUM_KEY_CODE_BROWSER_REFRESH;
        case VK_BROWSER_STOP:
            return GERIUM_KEY_CODE_BROWSER_STOP;
        case VK_BROWSER_SEARCH:
            return GERIUM_KEY_CODE_BROWSER_SEARCH;
        case VK_BROWSER_FAVORITES:
            return GERIUM_KEY_CODE_BROWSER_FAVORITES;
        case VK_BROWSER_HOME:
            return GERIUM_KEY_CODE_BROWSER_HOME;
        case VK_VOLUME_MUTE:
            return GERIUM_KEY_CODE_AUDIO_VOLUME_MUTE;
        case VK_VOLUME_DOWN:
            return GERIUM_KEY_CODE_AUDIO_VOLUME_DOWN;
        case VK_VOLUME_UP:
            return GERIUM_KEY_CODE_AUDIO_VOLUME_UP;
        case VK_MEDIA_NEXT_TRACK:
            return GERIUM_KEY_CODE_MEDIA_TRACK_NEXT;
        case VK_MEDIA_PREV_TRACK:
            return GERIUM_KEY_CODE_MEDIA_TRACK_PREVIOUS;
        case VK_MEDIA_STOP:
            return GERIUM_KEY_CODE_MEDIA_STOP;
        case VK_MEDIA_PLAY_PAUSE:
            return GERIUM_KEY_CODE_MEDIA_PLAY_PAUSE;
        case VK_LAUNCH_MAIL:
            return GERIUM_KEY_CODE_LAUNCH_MAIL;
        case VK_LAUNCH_MEDIA_SELECT:
            return GERIUM_KEY_CODE_LAUNCH_MEDIA_PLAYER;
        case VK_LAUNCH_APP1:
            return GERIUM_KEY_CODE_LAUNCH_APPLICATION_1;
        case VK_LAUNCH_APP2:
            return GERIUM_KEY_CODE_LAUNCH_APPLICATION_2;
        case VK_OEM_1:
            return shift ? GERIUM_KEY_CODE_COLON : GERIUM_KEY_CODE_SEMICOLON;
        case VK_OEM_PLUS:
            return GERIUM_KEY_CODE_ADD;
        case VK_OEM_COMMA:
            return shift ? GERIUM_KEY_CODE_LESS : GERIUM_KEY_CODE_COMMA;
        case VK_OEM_MINUS:
            return shift ? GERIUM_KEY_CODE_UNDERSCORE : GERIUM_KEY_CODE_SUBTRACT;
        case VK_OEM_PERIOD:
            return shift ? GERIUM_KEY_CODE_GREATER : GERIUM_KEY_CODE_PERIOD;
        case VK_OEM_2:
            return shift ? GERIUM_KEY_CODE_QUESTION : GERIUM_KEY_CODE_SLASH;
        case VK_OEM_3:
            return shift ? GERIUM_KEY_CODE_TILDE :  GERIUM_KEY_CODE_BACKQUOTE;
        case VK_OEM_4:
            return shift ? GERIUM_KEY_CODE_BRACE_LEFT : GERIUM_KEY_CODE_BRACKET_LEFT;
        case VK_OEM_5:
            return shift ? GERIUM_KEY_CODE_PIPE : GERIUM_KEY_CODE_BACKSLASH;
        case VK_OEM_6:
            return shift ? GERIUM_KEY_CODE_BRACE_RIGHT : GERIUM_KEY_CODE_BRACKET_RIGHT;
        case VK_OEM_7:
            return shift ? GERIUM_KEY_CODE_DOUBLE_QUOTE : GERIUM_KEY_CODE_QUOTE;
        case VK_OEM_8:
        case VK_OEM_102:
        case VK_PROCESSKEY:
        case VK_PACKET:
        case VK_ATTN:
        case VK_CRSEL:
        case VK_EXSEL:
        case VK_EREOF:
        case VK_PLAY:
        case VK_ZOOM:
        case VK_NONAME:
        case VK_PA1:
        case VK_OEM_CLEAR:
            return GERIUM_KEY_CODE_UNKNOWN;
    }

    return GERIUM_KEY_CODE_UNKNOWN;
}

} // namespace gerium::windows

#endif
