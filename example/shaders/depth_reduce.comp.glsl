#version 450

layout(set = 0, binding = 0) uniform sampler2D depth;

layout(set = 0, binding = 1, r32f) uniform writeonly image2D reduce;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

void main() {
    ivec2 texelPosition00 = ivec2(gl_GlobalInvocationID.xy) * 2;
    ivec2 texelPosition01 = texelPosition00 + ivec2(0, 1);
    ivec2 texelPosition10 = texelPosition00 + ivec2(1, 0);
    ivec2 texelPosition11 = texelPosition00 + ivec2(1, 1);
    
    float color00 = texelFetch(depth, texelPosition00, 0).r;
    float color01 = texelFetch(depth, texelPosition01, 0).r;
    float color10 = texelFetch(depth, texelPosition10, 0).r;
    float color11 = texelFetch(depth, texelPosition11, 0).r;

    float result = max(max(max(color00, color01), color10), color11);

    imageStore(reduce, ivec2(gl_GlobalInvocationID.xy), vec4(result, 0.0, 0.0, 0.0));

    // groupMemoryBarrier();
    // barrier();
}
