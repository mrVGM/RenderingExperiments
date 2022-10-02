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

cbuffer LightsInfo : register(b0)
{
    float m_numLights;
    float3 m_ambientLight;
    float m_ambientIntensity;
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
    float3 normal = p_normalTexture.Sample(p_sampler, float2(input.uv.x, 1 - input.uv.y)).xyz;
    if (dot(normal, normal) <= 0) {
        return float4(0.0, 0.2, 0.4, 1.0);
    }

    float3 diffuse = p_diffuseTexture.Sample(p_sampler, float2(input.uv.x, 1 - input.uv.y)).xyz;
    float3 position = p_positionTexture.Sample(p_sampler, float2(input.uv.x, 1 - input.uv.y)).xyz;

    float3 color = m_ambientIntensity * diffuse * m_ambientLight;

    for (int i = 0; i < m_numLights; ++i) {
        LightData cur = m_lights[i];
        float3 lightDir = cur.m_position - position;
        lightDir = normalize(lightDir);

        float diffuseCoef = dot(normal, lightDir);

        if (diffuseCoef > 0) {
            color += cur.m_intensity * diffuseCoef * diffuse * cur.m_color;
        }
    }

    return float4(color, 1);
}
