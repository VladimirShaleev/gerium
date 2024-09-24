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

#endif
