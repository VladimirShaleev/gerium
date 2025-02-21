#ifndef COMPONENTS_VEHICLE_HPP
#define COMPONENTS_VEHICLE_HPP

#include "../Common.hpp"

struct Vehicle {
    enum WheelDrive {
        FrontWheelDrive,
        RearWheelDrive,
        AllWheelDrive
    };

    std::vector<entt::entity> wheels{};
    gerium_float32_t frontSuspensionSidewaysAngle{ 0.0f };
    gerium_float32_t frontSuspensionForwardAngle{ 0.0f };
    gerium_float32_t frontKingPinAngle{ 0.0f };
    gerium_float32_t frontCasterAngle{ 0.0f };
    gerium_float32_t frontCamber{ 0.0f };
    gerium_float32_t frontToe{ 0.0f };
    gerium_float32_t rearSuspensionSidewaysAngle{ 0.0f };
    gerium_float32_t rearSuspensionForwardAngle{ 0.0f };
    gerium_float32_t rearKingPinAngle{ 0.0f };
    gerium_float32_t rearCasterAngle{ 0.0f };
    gerium_float32_t rearCamber{ 0.0f };
    gerium_float32_t rearToe{ 0.0f };
    gerium_float32_t frontSuspensionMinLength{ 0.3f };
    gerium_float32_t frontSuspensionMaxLength{ 0.5f };
    gerium_float32_t frontSuspensionFrequency{ 1.5f };
    gerium_float32_t frontSuspensionDamping{ 0.5f };
    gerium_float32_t rearSuspensionMinLength{ 0.3f };
    gerium_float32_t rearSuspensionMaxLength{ 0.5f };
    gerium_float32_t rearSuspensionFrequency{ 1.5f };
    gerium_float32_t rearSuspensionDamping{ 0.5f };
    gerium_float32_t maxRollAngle{ PI };
    gerium_float32_t maxHandBrakeTorque{ 4000.0f };
    gerium_float32_t maxSteeringAngle{ glm::radians(30.0f) };
    gerium_float32_t angularDamping{ 0.8f };
    gerium_float32_t antiRollbar{ true };
    gerium_float32_t maxTorque{ 500.0f };
    gerium_float32_t minRPM{ 1000.0f };
    gerium_float32_t maxRPM{ 6000.0f };
    gerium_float32_t switchTime{ 0.5f };
    gerium_float32_t clutchStrength{ 10.0f };
    gerium_float32_t limitedSlipRatio{ 1.4f };
    std::vector<gerium_float32_t> gearRatios{ 2.66f, 1.78f, 1.3f, 1.0f, 0.74f };
    WheelDrive wheelDrive{ FrontWheelDrive };
    WheelDrive handBrake{ FrontWheelDrive };
    gerium_float32_t previousForward{ 1.0f };
    gerium_float32_t forward{ 0.0f };
    gerium_float32_t right{ 0.0f };
    bool handBrakePressed{ false };
};

#endif
