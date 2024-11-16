#version 450

#include "common/types.h"
#include "common/utils.h"

const float EE = 1000.0;
const float e = 2.71828182845904523536028747135266249775724709369995957;
const float cutoffAngle = 1.6110731556870734;
const float steepness = 1.5;
const float rayleighZenithLength = 8.4e3;
const float mieZenithLength = 1.25e3;
const vec3  up = vec3(0.0, 1.0, 0.0);
const vec3  totalRayleigh = vec3(5.804542996261093e-6, 1.3562911419845635e-5, 3.0265902468824876e-5);
const vec3  mieConst = vec3(1.8399918514433978e14, 2.7798023919660528e14, 4.0790479543861094e14);
const float threeOverSixteenPi = 0.05968310365946075;
const float oneOverFourPi = 0.07957747154594767;
const float sunAngularDiameterCos = 0.999956676946448443553574619906976478926848692873900859324;

layout(local_size_x = SKY_GROUP_SIZE, local_size_y = SKY_GROUP_SIZE, local_size_z = 1) in;

layout(set = 0, binding = 0, std140) uniform SkyDataUBO {
    SkyData skyData;
};

layout(set = 0, binding = 1, rgba16f) uniform writeonly image2DArray envCube;

vec3 getDirection(uint face, float x, float y) {
    vec3 direction = vec3(0.0);
    switch (face) {
    case 0:
        direction = vec3(1.0, y, -x);
        break;
    case 1:
        direction = vec3(-1.0, y, x);
        break;
    case 2:
        direction = vec3(x, 1.0, -y);
        break;
    case 3:
        direction = vec3(x, -1.0, y);
        break;
    case 4:
        direction = vec3(x, y, 1.0);
        break;
    case 5:
        direction = vec3(-x, y, -1.0);
        break;
    }
    return normalize(direction);
}

float sunIntensity(float zenithAngleCos) {
    zenithAngleCos = clamp(zenithAngleCos, -1.0, 1.0);
    return EE * max(0.0, 1.0 - pow(e, -((cutoffAngle - acos(zenithAngleCos)) / steepness)));
}

vec3 totalMie(float t) {
    float c = (0.2 * t) * 10E-18;
    return 0.434 * c * mieConst;
}

float rayleighPhase(float cosTheta) {
    return threeOverSixteenPi * (1.0 + pow(cosTheta, 2.0));
}

float hgPhase(float cosTheta, float g) {
    float g2      = pow(g, 2.0);
    float inverse = 1.0 / pow(abs(1.0 - 2.0 * g * cosTheta + g2), 1.5);
    return oneOverFourPi * ((1.0 - g2) * inverse);
}

vec3 calcSkyColor(vec3 direction) {
    float vSunE = sunIntensity(dot(skyData.sunDirection.xyz, up));

    float vSunfade = 1.0 - clamp(1.0 - exp((skyData.sunDirection.y)), 0.0, 1.0);

    float rayleighCoefficient = skyData.rayleigh - (1.0 * (1.0 - vSunfade));

    vec3 vBetaR = totalRayleigh * rayleighCoefficient;

    vec3 vBetaM = totalMie(skyData.turbidity) * skyData.mieCoefficient;

    float zenithAngle = acos(max(0.0, dot(up, direction)));
    float inverse     = 1.0 / (cos(zenithAngle) + 0.15 * pow(abs(93.885 - ((zenithAngle * 180.0) / PI)), -1.253));
    float sR          = rayleighZenithLength * inverse;
    float sM          = mieZenithLength * inverse;

    vec3 Fex = exp(-(vBetaR * sR + vBetaM * sM));

    float cosTheta = dot(direction, skyData.sunDirection.xyz);

    float rPhase     = rayleighPhase(cosTheta * 0.5 + 0.5);
    vec3  betaRTheta = vBetaR * rPhase;

    float mPhase     = hgPhase(cosTheta, skyData.mieDirectionalG);
    vec3  betaMTheta = vBetaM * mPhase;

    vec3 Lin = pow(abs(vSunE * ((betaRTheta + betaMTheta) / (vBetaR + vBetaM)) * (1.0 - Fex)), vec3(1.5, 1.5, 1.5));
    Lin *= mix(
        vec3(1.0, 1.0, 1.0),
        pow(vSunE * ((betaRTheta + betaMTheta) / (vBetaR + vBetaM)) * Fex, vec3(1.0 / 2.0, 1.0 / 2.0, 1.0 / 2.0)),
        clamp(pow(1.0 - dot(up, skyData.sunDirection.xyz), 5.0), 0.0, 1.0));

    float theta = acos(direction.y);
    float phi   = atan(direction.z, direction.x);
    vec3  L0    = vec3(0.1, 0.1, 0.1) * Fex;

    float sundisk = smoothstep(sunAngularDiameterCos, sunAngularDiameterCos + 0.00002, cosTheta);
    L0 += (vSunE * 19000.0 * Fex) * sundisk;

    vec3 texColor = (Lin + L0) * 0.04 + vec3(0.0, 0.0003, 0.00075);
    vec3 color    = (log2(2.0 / pow(skyData.luminance, 4.0))) * texColor;
    vec3 retColor = pow(abs(color), vec3(1.0 / (1.2 + (1.2 * vSunfade))));

    return retColor;
}

void main() {
    vec2 uv = uvFromCoords(gl_GlobalInvocationID.xy, vec2(1.0 / 512));

    vec3 dir = getDirection(gl_GlobalInvocationID.z, 2.0 * uv.x - 1.0, 1.0 - 2.0 * uv.y);

    ivec3 coord = ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, gl_GlobalInvocationID.z);
    imageStore(envCube, coord, vec4(calcSkyColor(dir), 1.0));
}
