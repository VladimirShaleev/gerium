struct Output {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

Output main(uint id : SV_VertexID) {
    Output output;
    output.uv.x = float((id << 1) & 2);
    output.uv.y = float(id & 2);
    output.pos.xy = output.uv.xy * 2.0f - 1.0f;
    output.pos.z = 0.0f;
    output.pos.w = 1.0f;
    output.pos.y = -output.pos.y;
    return output;
}
