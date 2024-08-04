#ifndef GERIUM_WINDOWS_DIRECT_INPUT_HPP
#define GERIUM_WINDOWS_DIRECT_INPUT_HPP

#define DIRECTINPUT_VERSION 0x0800

#include "../Input.hpp"
#include "../Logger.hpp"
#include "Win32Application.hpp"

#include <dinput.h>
#include <dinputd.h>
#include <oleauto.h>

namespace gerium::windows {

class DirectInput final : public Input {
public:
    DirectInput(Win32Application* application);
    ~DirectInput() override;

private:
    void onPoll() override;
    bool onIsPressScancode(gerium_scancode_t scancode) const noexcept override;

    void createKeyboard(HWND hWnd);
    
    static bool devicePoll(LPDIRECTINPUTDEVICE8W device) noexcept;
    static uint8_t mapScanCode(gerium_scancode_t scancode) noexcept;

    ObjectPtr<Logger> _logger{};
    LPDIRECTINPUT8W _directInput{};
    LPDIRECTINPUTDEVICE8W _keyboard{};

    BYTE _keyState[256]{};
};

} // namespace gerium::windows

#endif
