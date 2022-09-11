cbuffer UVOffset : register(b0)
{
    float offset;
    float m_padding[63];
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : UV;
};

PSInput VSMain(float4 position : POSITION, float4 normal : NORMAL, float2 uv : UV)
{
    PSInput result;

    result.position = position;
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(1, 0, 1, 1);
}
