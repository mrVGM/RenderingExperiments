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

PSInput VSMain(float4 position : POSITION, float4 normal : NORMAL, float2 uv : UV)
{
    PSInput result;

    result.position = mul(m_matrix, position);
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(1, 0, 1, 1);
}
