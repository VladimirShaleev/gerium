#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "Common.hpp"

enum SettingsOutput {
    FinalResult     = OUTPUT_FINAL_RESULT,
    DirectLightOnly = OUTPUT_DIRECT_LIGHT_ONLY,
    // Albedo      = OUTPUT_ALBEDO,
    // Normal      = OUTPUT_NORMAL,
    // Metalness   = OUTPUT_METALNESS,
    // Roughness   = OUTPUT_ROUGHNESS,
    // Motion      = OUTPUT_MOTION,
    RadianceCache   = OUTPUT_RADIANCE_CACHE,
    IrradianceCache = OUTPUT_IRRADIANCE_CACHE,
    Meshlets        = OUTPUT_MESHLETS
};

struct Settings {
    bool DebugCamera      = false;
    bool MoveDebugCamera  = false;
    SettingsOutput Output = SettingsOutput::FinalResult;
    int Hour              = 7;
    float CurrentHour     = 7;

    bool isDebugGI() const noexcept {
        return Output == SettingsOutput::RadianceCache || Output == SettingsOutput::IrradianceCache;
    }
};

#endif
