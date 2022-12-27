cbuffer MVCMatrix : register(b0)
{
    float4x4 m_matrix;
    float4 m_camPos;
    float4 m_camRight;
    float4 m_camFwd;
    float4 m_camUp;

    float m_camFov;
    float m_aspect;
    float m_time;
    float m_airAbsorbtion;

    float m_sunAzimuth;
    float m_sunAltitude;
};

cbuffer CardData : register(b1)
{
    float m_texChannel;
};

Texture3D p_texture     : register(t0);
SamplerState p_sampler  : register(s0);

struct PSInput
{
    float4 position         : SV_POSITION;
    float2 uv               : UV;
};

PSInput VSMain(
    float2 position : POSITION,
    float3 objectPosition : OBJECT_POSITION,
    float4 objectRotation : OBJECT_ROTATION,
    float3 objectScale : OBJECT_SCALE)
{
    PSInput result;

    float3 pos = objectScale * float3(position, 0);
    pos += objectPosition;

    result.position = float4(pos, 1);
    result.uv = float2((position.x + 1) / 2, (position.y + 1) / 2);
    return result;
}

float4 PSMain(float4 position : SV_POSITION, float2 uv : UV) : SV_Target
{
    float4 tex = p_texture.Sample(p_sampler, float3(uv, 0.1 * m_time));

    int texChannel = (int)m_texChannel;

    return float4(tex[texChannel], tex[texChannel], tex[texChannel], 1);
}

