#version 450

#include "common/types.h"
#include "common/utils.h"

layout(location = 0) in vec2 texCoord;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

layout(binding = 0, set = 1) uniform sampler2D texAlbedo;
layout(binding = 1, set = 1) uniform sampler2D texNormal;
layout(binding = 2, set = 1) uniform sampler2D texMetallicRoughness;
layout(binding = 3, set = 1) uniform sampler2D texDepth;

layout(binding = 0, set = 2) uniform Bins {
    uint bins[LIGHT_Z_BINS];
};

layout(std430, binding = 1, set = 2) readonly buffer Lights {
    PointLight lights[];
};

layout(std430, binding = 2, set = 2) readonly buffer Tiles {
    uint tiles[];
};

layout(std430, binding = 3, set = 2) readonly buffer LightIndices {
    uint lightIndices[];
};

layout(location = 0) out vec4 outColor;

float attenuationSquareFalloff(vec3 positionToLight, float lightInverseRadius) {
    const float distanceSquare = dot(positionToLight, positionToLight);
    const float factor = distanceSquare * lightInverseRadius * lightInverseRadius;
    const float smoothFactor = max(1.0 - factor * factor, 0.0);
    return (smoothFactor * smoothFactor) / max(distanceSquare, 1e-4);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = geometrySchlickGGX(NdotV, roughness);
    float ggx1  = geometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
} 

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   

void main() {
    vec3 albedo = textureLod(texAlbedo, texCoord, 0).rgb;
    vec3 normal = octahedralDecode(texture(texNormal, texCoord).rg);
    vec3 orm = textureLod(texMetallicRoughness, texCoord, 0).rgb;
    vec3 position = worldPositionFromDepth(texCoord, texture(texDepth, texCoord).r, scene.invViewProjection);
    float ao = orm.r;
    float roughness = orm.g;
    float metallic = orm.b;

    vec3 N = normalize(normal);
    vec3 V = normalize(scene.viewPosition.xyz - position);
    vec3 R = reflect(-V, N); 

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0);

    float zFar = scene.planes.x;
    float zNear = scene.planes.y;

    vec4 posCameraSpace = scene.view * vec4(position, 1.0);

    float linearD = (posCameraSpace.z - zNear) / (zFar - zNear);
    int binIndex = int(linearD * LIGHT_Z_BINS);
    uint binValue = bins[binIndex];

    uint minLightId = binValue & 0xFFFF;
    uint maxLightId = (binValue >> 16) & 0xFFFF;

    uvec2 pixelPos = uvec2(gl_FragCoord.x - 0.5, gl_FragCoord.y - 0.5);

    uvec2 tile = pixelPos / uint(TILE_SIZE);

    uint stride = uint(NUM_WORDS) * (uint(scene.resolution.x) / uint(TILE_SIZE));
    uint address = tile.y * stride + tile.x;

    if (minLightId != MAX_LIGHTS + 1) {
        for (uint lightId = minLightId; lightId <= maxLightId; ++lightId) {
            uint wordId = lightId / 32;
            uint bitId = lightId % 32;
    
            if ((tiles[address + wordId] & (1 << bitId)) != 0) {
                uint globalLightIndex = lightIndices[lightId];
                PointLight pointLight = lights[globalLightIndex];

                vec3 L = normalize(pointLight.position.xyz - position);
                vec3 H = normalize(V + L);

                const vec3 positionToLight = pointLight.position.xyz - position;
                float attenuation = attenuationSquareFalloff(positionToLight, 1.0f / pointLight.attenuation);
                vec3 radiance     = pointLight.color.rgb * attenuation * pointLight.intensity;        

                // Cook-Torrance BRDF
                float NDF = distributionGGX(N, H, roughness);        
                float G   = geometrySmith(N, V, L, roughness);      
                vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);       
            
                vec3 kS = F;
                vec3 kD = vec3(1.0) - kS;
                kD *= 1.0 - metallic;	  
            
                vec3  numerator   = NDF * G * F;
                float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
                vec3  specular    = numerator / max(denominator, 0.001);  
                
                float NdotL = max(dot(N, L), 0.0);                
                Lo += (kD * albedo / PI + specular) * radiance * NdotL;
           }
        }
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
  
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    vec3 irradiance = vec3(1.0); // texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = vec3(1.0); // textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = vec2(1.0); // texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = vec3(0.0); // (kD * diffuse + specular) * ao * ao;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}
