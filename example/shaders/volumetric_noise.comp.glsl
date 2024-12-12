#version 450

layout(set = 0, binding = 0, r8) uniform writeonly image3D noise;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

vec3 interpolationC2(vec3 x) {
    return x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
}

// from: https://github.com/BrianSharpe/GPU-Noise-Lib/blob/master/gpu_noise_lib.glsl
void perlinHash(vec3 gridcell, float s, bool tile, 
                out vec4 lowzHash0,
                out vec4 lowzHash1,
                out vec4 lowzHash2,
                out vec4 highzHash0,
                out vec4 highzHash1,
                out vec4 highzHash2)
{
    const vec2  OFFSET          = vec2(50.0, 161.0);
    const float DOMAIN          = 69.0;
    const vec3  SOMELARGEFLOATS = vec3(635.298681, 682.357502, 668.926525);
    const vec3  ZINC            = vec3(48.500388, 65.294118, 63.934599);

    gridcell.xyz = gridcell.xyz - floor(gridcell.xyz * (1.0 / DOMAIN)) * DOMAIN;
    float d = DOMAIN - 1.5;
    vec3 gridcellInc1 = step(gridcell, vec3(d, d, d)) * (gridcell + 1.0);

    gridcellInc1 = tile ? mod(gridcellInc1, s) : gridcellInc1;

    vec4 P = vec4(gridcell.xy, gridcellInc1.xy) + OFFSET.xyxy;
    P *= P;
    P = P.xzxz * P.yyww;
    vec3 lowzMod = vec3( 1.0 / (SOMELARGEFLOATS.xyz + gridcell.zzz * ZINC.xyz));
    vec3 highzMod = vec3( 1.0 / (SOMELARGEFLOATS.xyz + gridcellInc1.zzz * ZINC.xyz));

    lowzHash0  = fract(P * lowzMod.xxxx);
    highzHash0 = fract(P * highzMod.xxxx);
    lowzHash1  = fract(P * lowzMod.yyyy);
    highzHash1 = fract(P * highzMod.yyyy);
    lowzHash2  = fract(P * lowzMod.zzzz);
    highzHash2 = fract(P * highzMod.zzzz);
}

// https://github.com/BrianSharpe/GPU-Noise-Lib/blob/master/gpu_noise_lib.glsl
float perlin(vec3 P, float s, bool tile) {
    P *= s;

    vec3 Pi     = floor(P);
    vec3 Pi2    = floor(P);
    vec3 Pf     = P - Pi;
    vec3 pfMin1 = Pf - 1.0;

    vec4 hashx0, hashy0, hashz0, hashx1, hashy1, hashz1;
    perlinHash(Pi2, s, tile, hashx0, hashy0, hashz0, hashx1, hashy1, hashz1);

    vec4 gradX0 = hashx0 - 0.49999;
    vec4 gradY0 = hashy0 - 0.49999;
    vec4 gradZ0 = hashz0 - 0.49999;
    vec4 gradX1 = hashx1 - 0.49999;
    vec4 gradY1 = hashy1 - 0.49999;
    vec4 gradZ1 = hashz1 - 0.49999;
    vec4 gradResults0 = 1.0 / sqrt(gradX0 * gradX0 + gradY0 * gradY0 + gradZ0 * gradZ0) * (vec2(Pf.x, pfMin1.x).xyxy * gradX0 + vec2(Pf.y, pfMin1.y).xxyy * gradY0 + Pf.zzzz * gradZ0);
    vec4 gradResults1 = 1.0 / sqrt(gradX1 * gradX1 + gradY1 * gradY1 + gradZ1 * gradZ1) * (vec2(Pf.x, pfMin1.x).xyxy * gradX1 + vec2(Pf.y, pfMin1.y).xxyy * gradY1 + pfMin1.zzzz * gradZ1);

    vec3 blend = interpolationC2( Pf );
    vec4 res0 = mix(gradResults0, gradResults1, blend.z);
    vec4 blend2 = vec4(blend.xy, vec2(1.0 - blend.xy));
    float final = dot(res0, blend2.zxzx * blend2.wwyy);
    final *= 1.0 / sqrt(0.75);
    return ((final * 1.5) + 1.0) * 0.5;
}

float perlin(vec3 P) {
    return perlin(P, 1, false);
}

float perlin7Octaves(vec3 xyz, float s) {
    float f = 1.0;
    float a = 1.0;

    float perlinValue = 0.0;
    perlinValue += a * perlin(xyz, s * f, true).r; a *= 0.5; f *= 2.0;
    perlinValue += a * perlin(xyz, s * f, true).r; a *= 0.5; f *= 2.0;
    perlinValue += a * perlin(xyz, s * f, true).r; a *= 0.5; f *= 2.0;
    perlinValue += a * perlin(xyz, s * f, true).r; a *= 0.5; f *= 2.0;
    perlinValue += a * perlin(xyz, s * f, true).r; a *= 0.5; f *= 2.0;
    perlinValue += a * perlin(xyz, s * f, true).r; a *= 0.5; f *= 2.0;
    perlinValue += a * perlin(xyz, s * f, true).r;

    return perlinValue;
}

void main() {
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);

    float perlinData = perlin7Octaves(pos / 64.0, 8.0);

    imageStore(noise, pos, vec4(perlinData, 0.0, 0.0, 0.0));
}
