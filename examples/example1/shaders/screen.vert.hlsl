struct Output {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

Output main(uint id : SV_VertexID) {
    Output output;
    output.uv    = float2((id << 1) & 2, id & 2);
    output.pos   = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);
    output.pos.y = -output.pos.y;
    return output;
}
