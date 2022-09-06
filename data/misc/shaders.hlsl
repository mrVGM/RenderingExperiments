cbuffer UVOffset : register(b0)
{
    float2 uvOffset;
    float m_padding[62];
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : UV;
};

Texture3D p_texture     : register(t0);
SamplerState p_sampler  : register(s0);

PSInput VSMain(float2 position : POSITION, float2 uv : UV)
{
    PSInput result;

    result.position = float4(position, 0, 1);
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return p_texture.Sample(p_sampler, float3(input.uv + uvOffset, 0.5));
}
