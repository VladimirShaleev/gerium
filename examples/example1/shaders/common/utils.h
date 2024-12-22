#ifndef UTILS_H
#define UTILS_H

vec2 signNotZero(vec2 v) {
    return vec2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
}

vec2 octahedralEncode(vec3 v) {
    vec2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
    return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * signNotZero(p)) : p;
}

vec3 octahedralDecode(vec2 f) {
    vec3 n  = vec3(f.xy, 1.0 - abs(f.x) - abs(f.y));
    float t = max(-n.z, 0.0);
    n.x += n.x >= 0.0 ? -t : t;
    n.y += n.y >= 0.0 ? -t : t;
    return normalize(n);
}

vec3 worldPositionFromDepth(vec2 uv, float rawDepth, mat4 invViewProjection) {
    vec4 pos = vec4(uv.x * 2.0 - 1.0, uv.y * -2.0 + 1.0, rawDepth, 1.0);
    vec4 worldPos = invViewProjection * pos;

    return worldPos.xyz / worldPos.w;
}

float filterCubic(float x, float B, float C) {
    float y = 0.0;
    float x2 = x * x;
    float x3 = x2 * x;
    if(x < 1) {
        y = (12.0 - 9.0 * B - 6.0 * C) * x3 + (-18.0 + 12.0 * B + 6.0 * C) * x2 + (6.0 - 2.0 * B);
    } else if (x <= 2) {
        y = (-B - 6.0 * C) * x3 + (6.0 * B + 30.0 * C) * x2 + (-12.0 * B - 48.0 * C) * x + (8.0 * B + 24.0 * C);
    }
    return y / 6.0;
}

float filterCatmullRom(float value) {
    return filterCubic(value, 0.0, 0.5);
}

float subsampleFilter(float value) {
    return filterCatmullRom(value * 2.0);
}

vec4 clipAABB(vec3 aabbMin, vec3 aabbMax, vec4 previousSample, float averageAlpha) {
    vec3 pClip = 0.5 * (aabbMax + aabbMin);
    vec3 eClip = 0.5 * (aabbMax - aabbMin) + 0.000000001;

    vec4 vClip = previousSample - vec4(pClip, averageAlpha);
    vec3 vUnit = vClip.xyz / eClip;
    vec3 aUnit = abs(vUnit);
    float maUnit = max(aUnit.x, max(aUnit.y, aUnit.z));

    if (maUnit > 1.0) {
        return vec4(pClip, averageAlpha) + vClip / maUnit;
    } else {
        return previousSample;
    }
}

vec3 varianceClipClamp(vec3 historyColor, vec3 neighborhoodMin, vec3 neighborhoodMax, vec3 m1, vec3 m2) {
    const float rcpSampleCount = 1.0 / 9.0;
    const float gamma = 1.0;
    vec3 mu = m1 * rcpSampleCount;
    vec3 sigma = sqrt(abs((m2 * rcpSampleCount) - (mu * mu)));
    vec3 minc = mu - gamma * sigma;
    vec3 maxc = mu + gamma * sigma;

    vec3 clampedHistoryColor = clamp(historyColor, neighborhoodMin, neighborhoodMax);
    return clipAABB(minc, maxc, vec4(clampedHistoryColor, 1.0), 1.0).rgb;
}

#endif
