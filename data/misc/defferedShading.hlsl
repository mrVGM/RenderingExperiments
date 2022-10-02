struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : UV;
};

struct LightData
{
    float3 m_position;
    float3 m_color;
    float m_intensity;
};

StructuredBuffer<LightData> m_lights : register(t0);

Texture2D p_diffuseTexture  : register(t1);
Texture2D p_normalTexture   : register(t2);
Texture2D p_positionTexture : register(t3);

SamplerState p_sampler      : register(s0);

PSInput VSMain(float2 position : POSITION, float2 uv : UV)
{
    PSInput result;

    result.position = float4(0.5 * position, 0, 1);
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return p_normalTexture.Sample(p_sampler, float2(input.uv.x, 1 - input.uv.y));
}
