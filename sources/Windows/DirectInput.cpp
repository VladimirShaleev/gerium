#include "DirectInput.hpp"

namespace gerium {

namespace windows {

DirectInput::DirectInput(Win32Application* application) {
    _logger = Logger::create("gerium:input:windows");

    if (FAILED(DirectInput8Create(
            GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*) &_directInput, nullptr))) {
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
    }

    createKeyboard(application->hWnd());
}

void DirectInput::onPoll() {
    if (_keyboard && devicePoll(_keyboard)) {
        if (FAILED(_keyboard->GetDeviceState(256, (LPVOID) _keyState))) {
            _logger->print(GERIUM_LOGGER_LEVEL_WARNING, "Poll keyboard state failed");
        }
    }
}

bool DirectInput::onIsPressScancode(gerium_scancode_t scancode) const noexcept {
    const auto index = mapScanCode(scancode);
    return index != 0 ? (_keyState[index] & 0x80) != 0 : false;
}

void DirectInput::createKeyboard(HWND hWnd) {
    LPDIRECTINPUTDEVICE8W device;
    if (FAILED(_directInput->CreateDevice(GUID_SysKeyboard, &device, nullptr))) {
        return;
    }

    if (FAILED(device->SetDataFormat(&c_dfDIKeyboard))) {
        return;
    }

    if (FAILED(device->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND))) {
        return;
    }

    _keyboard = device;
}

bool DirectInput::devicePoll(LPDIRECTINPUTDEVICE8W device) noexcept {
    if (auto hr = device->Poll(); FAILED(hr)) {
        switch (hr) {
            case DIERR_INPUTLOST:
                do {
                    hr = device->Acquire();
                } while (hr == DIERR_INPUTLOST);
                return false;
            case DIERR_NOTACQUIRED:
                do {
                    hr = device->Acquire();
                } while (hr == DIERR_NOTACQUIRED);
                device->Poll();
                break;
        }
    }
    return true;
}

uint8_t DirectInput::mapScanCode(gerium_scancode_t scancode) noexcept {
    static constexpr uint8_t scancodes[] = { DIK_0,           DIK_1,
                                             DIK_2,           DIK_3,
                                             DIK_4,           DIK_5,
                                             DIK_6,           DIK_7,
                                             DIK_8,           DIK_9,
                                             DIK_A,           DIK_B,
                                             DIK_C,           DIK_D,
                                             DIK_E,           DIK_F,
                                             DIK_G,           DIK_H,
                                             DIK_I,           DIK_J,
                                             DIK_K,           DIK_L,
                                             DIK_M,           DIK_N,
                                             DIK_O,           DIK_P,
                                             DIK_Q,           DIK_R,
                                             DIK_S,           DIK_T,
                                             DIK_U,           DIK_V,
                                             DIK_W,           DIK_X,
                                             DIK_Y,           DIK_Z,
                                             DIK_F1,          DIK_F2,
                                             DIK_F3,          DIK_F4,
                                             DIK_F5,          DIK_F6,
                                             DIK_F7,          DIK_F8,
                                             DIK_F9,          DIK_F10,
                                             DIK_F11,         DIK_F12,
                                             DIK_F13,         DIK_F14,
                                             DIK_F15,         DIK_CALCULATOR,
                                             DIK_NUMPAD0,     DIK_NUMPAD1,
                                             DIK_NUMPAD2,     DIK_NUMPAD3,
                                             DIK_NUMPAD4,     DIK_NUMPAD5,
                                             DIK_NUMPAD6,     DIK_NUMPAD7,
                                             DIK_NUMPAD8,     DIK_NUMPAD9,
                                             DIK_NUMLOCK,     DIK_NUMPADCOMMA,
                                             DIK_NUMPADENTER, DIK_NUMPADEQUALS,
                                             DIK_NUMPADMINUS, DIK_NUMPADPERIOD,
                                             DIK_NUMPADPLUS,  DIK_NUMPADSLASH,
                                             DIK_NUMPADSTAR,  DIK_PLAYPAUSE,
                                             DIK_PREVTRACK,   DIK_NEXTTRACK,
                                             DIK_MUTE,        DIK_VOLUMEDOWN,
                                             DIK_VOLUMEUP,    DIK_MEDIASELECT,
                                             DIK_MEDIASTOP,   DIK_ESCAPE,
                                             DIK_TAB,         DIK_CAPSLOCK,
                                             DIK_RETURN,      DIK_BACKSLASH,
                                             DIK_BACKSPACE,   DIK_MINUS,
                                             DIK_COLON,       DIK_COMMA,
                                             DIK_CONVERT,     DIK_NOCONVERT,
                                             DIK_EQUALS,      DIK_GRAVE,
                                             DIK_PERIOD,      DIK_POWER,
                                             DIK_SEMICOLON,   DIK_SLASH,
                                             DIK_SLEEP,       DIK_SPACE,
                                             DIK_STOP,        DIK_SYSRQ,
                                             DIK_UNDERLINE,   DIK_UNLABELED,
                                             DIK_APOSTROPHE,  DIK_WAKE,
                                             DIK_LALT,        DIK_RALT,
                                             DIK_LBRACKET,    DIK_RBRACKET,
                                             DIK_LCONTROL,    DIK_RCONTROL,
                                             DIK_LSHIFT,      DIK_RSHIFT,
                                             DIK_LWIN,        DIK_RWIN,
                                             DIK_UPARROW,     DIK_LEFTARROW,
                                             DIK_RIGHTARROW,  DIK_DOWNARROW,
                                             DIK_SCROLL,      DIK_PAUSE,
                                             DIK_INSERT,      DIK_DELETE,
                                             DIK_HOME,        DIK_END,
                                             DIK_PGUP,        DIK_PGDN,
                                             DIK_MAIL,        DIK_MYCOMPUTER,
                                             DIK_APPS,        DIK_ABNT_C1,
                                             DIK_ABNT_C2,     DIK_AT,
                                             DIK_AX,          DIK_KANA,
                                             DIK_KANJI,       DIK_OEM_102,
                                             DIK_WEBBACK,     DIK_WEBFAVORITES,
                                             DIK_WEBFORWARD,  DIK_WEBHOME,
                                             DIK_WEBREFRESH,  DIK_WEBSEARCH,
                                             DIK_WEBSTOP,     DIK_YEN };
    if (scancode < std::size(scancodes)) {
        return scancodes[(int) scancode];
    }
    return 0;
}

DirectInput::~DirectInput() {
    if (_keyboard) {
        _keyboard->Release();
        _keyboard = nullptr;
    }

    if (_directInput) {
        _directInput->Release();
        _directInput = nullptr;
    }
}

} // namespace windows

ObjectPtr<Input> Input::create(Application* application) {
    Input* input = nullptr;
    error(Object::create<windows::DirectInput>(input, alias_cast<windows::Win32Application*>(application)));
    return ObjectPtr(input, false);
}

} // namespace gerium
