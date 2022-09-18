cbuffer MVCMatrix : register(b0)
{
    float4x4 m_matrix;
    float m_padding[48];
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : UV;
};

PSInput VSMain(float3 position : POSITION, float3 normal : NORMAL, float2 uv : UV, float3 objectPosition : OBJECT_POSITION)
{
    PSInput result;

    result.position = mul(m_matrix, float4 (objectPosition + position, 1));
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(1, 0, 1, 1);
}
