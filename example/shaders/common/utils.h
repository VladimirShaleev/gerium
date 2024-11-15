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

Angular calcAngular(vec3 pointToLight, vec3 n, vec3 v) {
    vec3 l = normalize(pointToLight);
    vec3 h = normalize(l + v);

    Angular angular;
    angular.NdotL = clamp(dot(n, l), 0.0, 1.0);
    angular.NdotV = clamp(dot(n, v), 0.0, 1.0);
    angular.NdotH = clamp(dot(n, h), 0.0, 1.0);
    angular.LdotH = clamp(dot(l, h), 0.0, 1.0);
    angular.VdotH = clamp(dot(v, h), 0.0, 1.0);
    return angular;
}

vec3 fresnelSchlick(PixelData pixelData, Angular angular) {
    return pixelData.F0 + (pixelData.F90 - pixelData.F0) * pow(clamp(1.0 - angular.VdotH, 0.0, 1.0), 5.0);
}

float visibilityOcclusionSmithJointGGX(PixelData pixelData, Angular angular) {
    float NdotL = angular.NdotL;
    float NdotV = angular.NdotV;
    float alphaRoughnessSq = pixelData.alphaRoughness * pixelData.alphaRoughness;

    float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
    float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

    float GGX = GGXV + GGXL;
    if (GGX > 0.0) {
        return 0.5 / GGX;
    }
    return 0.0;
}

float microfacetDistributionTrowbridge(PixelData pixelData, Angular angular) {
    float alphaRoughnessSq = pixelData.alphaRoughness * pixelData.alphaRoughness;
    float f = (angular.NdotH * alphaRoughnessSq - angular.NdotH) * angular.NdotH + 1.0;
    return alphaRoughnessSq / (PI * f * f + 0.000001f);
}

vec3 diffuseLambert(PixelData pixelData) {
    return pixelData.baseColor / PI;
}

void pointShadeSeparate(vec3 pointToLight, vec3 normal, vec3 view, PixelData pixelData, inout vec3 diffuse, inout vec3 specular) {
    Angular angular = calcAngular(pointToLight, normal, view);
    if (angular.NdotL > 0.0 || angular.NdotV > 0.0) {
        vec3  F = fresnelSchlick(pixelData, angular);
        float V = visibilityOcclusionSmithJointGGX(pixelData, angular);
        float D = microfacetDistributionTrowbridge(pixelData, angular);

        vec3 diffuseContrib = (1.0 - F) * diffuseLambert(pixelData);
        vec3 specContrib    = F * V * D;

        diffuse = angular.NdotL * diffuseContrib;
        specular = angular.NdotL * specContrib;
    }
}

void calcLightDirectional(vec3 normal, vec3 view, Light light, PixelData pixelData, inout vec3 diffuse, inout vec3 specular) {
    vec3 pointToLight = light.directionRange.xyz;

    vec3 shadeDiffuse = vec3(0.0);
    vec3 shaderSpecular = vec3(0.0);

    pointShadeSeparate(pointToLight, normal, view, pixelData, shadeDiffuse, shaderSpecular);

    diffuse = light.colorIntensity.rgb * light.colorIntensity.a * shadeDiffuse;
    specular = light.colorIntensity.rgb * light.colorIntensity.a * shaderSpecular;
}

void calcLight(vec3 worldPosition, vec3 normal, vec3 view, Light light, PixelData pixelData, inout vec3 diffuse, inout vec3 specular) {
    if (light.type == LIGHT_TYPE_DIRECTIONAL) {
        calcLightDirectional(normal, view, light, pixelData, diffuse, specular);
    } else if (light.type == LIGHT_TYPE_POINT) {
        // TODO:
    } else if (light.type == LIGHT_TYPE_SPOT) {
        // TODO:
    }
}

void lighting(vec3 worldPosition, vec3 normal, vec3 view, PixelData pixelData, inout vec3 color, inout vec3 colorDifffuse) {
    Light lights[1];
    lights[0].directionRange = vec4(-1.0, 1.0, -1.0, 1.0);
    lights[0].colorIntensity = vec4(1.0, 1.0, 1.0, 10.0);
    lights[0].type           = LIGHT_TYPE_DIRECTIONAL;

    for (int i = 0; i < 1; ++i) {
        vec3 diffuse = vec3(0.0);
        vec3 specular = vec3(0.0);

        calcLight(worldPosition, normal, view, lights[i], pixelData, diffuse, specular);

        color += diffuse + specular;
        colorDifffuse += diffuse;
    }

    // TODO: add IBL
}

#endif
