#ifndef UTILS_H
#define UTILS_H

bool coneCulling(vec3 center, float radius, vec3 coneAxis, float coneCutoff, vec3 cameraPosition) {
    return dot(center - cameraPosition, coneAxis) >= coneCutoff * length(center - cameraPosition) + radius;
}

bool projectSphere(vec3 c, float r, float znear, float P00, float P11, out vec4 aabb) {
    if (c.z < r + znear) {
        return false;
    }

    vec3 cr = c * r;
    float czr2 = c.z * c.z - r * r;

    float vx = sqrt(c.x * c.x + czr2);
    float minx = (vx * c.x - cr.z) / (vx * c.z + cr.x);
    float maxx = (vx * c.x + cr.z) / (vx * c.z - cr.x);

    float vy = sqrt(c.y * c.y + czr2);
    float miny = (vy * c.y - cr.z) / (vy * c.z + cr.y);
    float maxy = (vy * c.y + cr.z) / (vy * c.z - cr.y);

    aabb = vec4(minx * P00, miny * P11, maxx * P00, maxy * P11);
    aabb = aabb.xwzy * vec4(0.5, -0.5, 0.5, -0.5) + vec4(0.5);

    return true;
}

vec3 rotateQuaternion(vec3 v, vec4 q) {
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

uint hash(uint a) {
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23c) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09) ^ (a >> 16);
    return a;
}

vec3 worldPositionFromDepth(vec2 uv, float rawDepth, mat4 invViewProjection) {
    vec4 pos = vec4(uv.x * 2.0 - 1.0, uv.y * -2.0 + 1.0, rawDepth, 1.0);
    vec4 worldPos = invViewProjection * pos;

    return worldPos.xyz / worldPos.w;
}

vec2 uvFromCoords(uvec2 coord, vec2 texelSize) {
    return (coord + 0.5) * texelSize;
}

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

float sliceExponentialDepth(float near, float far, float jitter, float slice, float numSlices) {
    return near * pow(far / near, (slice + 0.5 + jitter) / numSlices);
}

float linearDepthToUv(float near, float far, float linearDepth, float numSlices) {
    const float oneOverLogFOverN = 1.0 / log2(far / near);
    const float scale = numSlices * oneOverLogFOverN;
    const float bias = -(numSlices * log2(near) * oneOverLogFOverN);
    return max(log2(linearDepth) * scale + bias, 0.0) / float(numSlices);
}

float linearDepthToRawDepth(float linearDepth, float near, float far) {
    return (near * far) / (linearDepth * (near - far)) - far / (near - far);
}

float rawDepthToLinearDepth(float rawDepth, float near, float far) {
    return near * far / (far + rawDepth * (near - far));
}

vec3 worldPositionFromFroxel(ivec3 froxelCoord, uvec3 dimensions, float near, float far, mat4 inverseVP, vec2 jitter) {
    vec2 texelSize = 1.0 / vec2(dimensions.x, dimensions.y);
    vec2 uv = (froxelCoord.xy + vec2(0.5) + jitter * 0.5) * texelSize;
    float linearDepth = sliceExponentialDepth(near, far, jitter.x * 0.5, float(froxelCoord.z), float(dimensions.z));

    float rawDepth = linearDepthToRawDepth(linearDepth, near, far);
    vec3 worldPosition = worldPositionFromDepth(uv, rawDepth, inverseVP);
    
    return worldPosition;
}

#endif
