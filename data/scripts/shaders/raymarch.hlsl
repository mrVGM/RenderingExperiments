cbuffer CamInfo : register(b0)
{
    float4 m_camPos;
    float4 m_bottomLeft;
    float4 m_bottomRight;
    float4 m_upLeft;
};

struct PSInput
{
    float4 position         : SV_POSITION;
    float2 uv               : UV;
};

PSInput VSMain(
    float2 position : POSITION)
{
    PSInput result;

    result.position = float4(position, 0, 1);
    result.uv = float2((position.x + 1) / 2, (position.y + 1) / 2);
    return result;
}

float4 PSMain(float4 position : SV_POSITION, float2 uv : UV) : SV_Target
{
    float3 target = uv.x * (m_bottomRight.xyz - m_bottomLeft.xyz);
    target += uv.y * (m_upLeft.xyz - m_bottomLeft.xyz);
    target += m_bottomLeft.xyz;

    float3 dir = normalize(target - m_camPos.xyz);

    return float4(dir,1);
}

