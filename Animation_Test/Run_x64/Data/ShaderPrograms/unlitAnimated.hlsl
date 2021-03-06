
cbuffer matrix_cb : register(b0) {
    float4x4 g_MODEL;
    float4x4 g_VIEW;
    float4x4 g_PROJECTION;
};

cbuffer time_cb : register(b1) {
    float g_GAME_TIME;
    float g_SYSTEM_TIME;
    float g_GAME_FRAME_TIME;
    float g_SYSTEM_FRAME_TIME;
}

struct vs_in_t {
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv : UV;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct ps_in_t {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : UV;
    float3 tangent : TANGENT;

};

SamplerState sSampler : register(s0);

Texture2DArray<float4> tDiffuse    : register(t0);
Texture2DArray<float4> tNormal   : register(t1);
Texture2DArray<float4> tDisplacement : register(t2);
Texture2DArray<float4> tSpecular : register(t3);
Texture2DArray<float4> tOcclusion : register(t4);
Texture2DArray<float4> tEmissive : register(t5);


ps_in_t VertexFunction(vs_in_t input_vertex) {
    ps_in_t output;

    float4 local = float4(input_vertex.position, 1.0f);
    float4 world = mul(local, g_MODEL);
    float4 view = mul(world, g_VIEW);
    float4 clip = mul(view, g_PROJECTION);

    output.position = clip;
    output.color = input_vertex.color;
    output.uv = input_vertex.uv;// * float2(1.0f, -1.0f);
    output.tangent = input_vertex.tangent;
    return output;
}

float4 PixelFunction(ps_in_t input_pixel) : SV_Target0 {
    input_pixel.tangent.x = 50;
    float4 albedo = tDiffuse.Sample(sSampler, float3(input_pixel.uv, input_pixel.tangent.x));
    return albedo * input_pixel.color;
}
