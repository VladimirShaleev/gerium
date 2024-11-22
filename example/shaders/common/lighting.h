#ifndef LIGHTING_H
#define LIGHTING_H

#include "types.h"

layout(binding = 0, set = 2) uniform sampler2D lutBRDF;

layout(binding = 1, set = 2) uniform sampler2DArrayShadow csm;

layout(std140, binding = 2, set = 2) uniform LightCountUBO {
    uint lightCount;
};

layout(std430, binding = 3, set = 2) readonly buffer LightSSBO {
    Light lights[];
};

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

vec3 contributionIBL(vec3 normal, vec3 view, PixelData pixelData, vec3 diffuseGI, vec3 specularGI, float diffuseGIFactor, float specularGIFactor) {
    float NdotV = clamp(dot(normal, view), 0.0, 1.0);
    vec2 brdfSamplePoint = clamp(vec2(NdotV, 1.0 - pixelData.perceptualRoughness), vec2(0.0), vec2(1.0));
    vec2 brdf = textureLod(lutBRDF, brdfSamplePoint, 0).rg;

    vec3 diffuse = diffuseGI * pixelData.baseColor * diffuseGIFactor;
    vec3 specular = specularGI * (pixelData.F0 * brdf.x + pixelData.F90 * brdf.y) * specularGIFactor;

    return diffuse + specular;
}

void lighting(vec3 worldPosition, vec3 normal, vec3 view, PixelData pixelData, vec3 diffuseGI, vec3 specularGI, inout vec3 color, inout vec3 colorDifffuse) {
    for (int i = 0; i < lightCount; ++i) {
        vec3 diffuse = vec3(0.0);
        vec3 specular = vec3(0.0);

        calcLight(worldPosition, normal, view, lights[i], pixelData, diffuse, specular);

        color += diffuse + specular;
        colorDifffuse += diffuse;
    }

    // color += contributionIBL(normal, view, pixelData, diffuseGI, specularGI, 3.0, 1.0);

    // colorDifffuse += contributionIBL(normal, view, pixelData, diffuseGI, specularGI, 3.0, 0.0);
}

#endif
