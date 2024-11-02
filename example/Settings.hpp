#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "Common.hpp"

enum SettingsOutput {
    FinalResult = OUTPUT_FINAL_RESULT,
    Meshlets    = OUTPUT_MESHLETS,
    Albedo      = OUTPUT_ALBEDO,
    Normal      = OUTPUT_NORMAL,
    Metalness   = OUTPUT_METALNESS,
    Roughness   = OUTPUT_ROUGHNESS,
    Motion      = OUTPUT_MOTION
};

struct Settings {
    bool DebugCamera;
    bool MoveDebugCamera;
    SettingsOutput Output;
};

#endif
