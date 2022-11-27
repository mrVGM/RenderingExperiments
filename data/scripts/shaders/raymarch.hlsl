cbuffer CamInfo : register(b0)
{
    float4 m_camPos;
    float4 m_bottomLeft;
    float4 m_bottomRight;
    float4 m_upLeft;
    float4 m_sphere;
};

struct PSInput
{
    float4 position         : SV_POSITION;
    float2 uv               : UV;
};

float sdf(float3 pos)
{
    return length(pos - m_sphere.xyz) - m_sphere.w;
}

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
    float t = sdf(m_camPos.xyz);
    if (t <= 0) {
        return float4(1, 1, 1, 1);
    }
    
    float3 target = uv.x * (m_bottomRight.xyz - m_bottomLeft.xyz);
    target += uv.y * (m_upLeft.xyz - m_bottomLeft.xyz);
    target += m_bottomLeft.xyz;

    float3 dir = normalize(target - m_camPos.xyz);

    float3 curPos = m_camPos;

    for (int i = 0; i < 100; ++i) {
        curPos += t * dir;

        t = sdf(curPos);

        if (t < 0.001) {
            return float4(1, 1, 0, 1);
        }
    }

    return float4(0,0,0,1);
}

