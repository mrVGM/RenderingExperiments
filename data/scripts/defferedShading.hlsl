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

cbuffer CamInfo : register(b1)
{
    float4x4 m_matrix;
    float3 m_camPos;
};

StructuredBuffer<LightData> m_lights : register(t0);

Texture2D p_diffuseTexture  : register(t1);
Texture2D p_normalTexture   : register(t2);
Texture2D p_positionTexture : register(t3);
Texture2D p_specularTexture : register(t4);

SamplerState p_sampler      : register(s0);

PSInput VSMain(float2 position : POSITION, float2 uv : UV)
{
    PSInput result;

    result.position = float4(position, 0, 1);
    result.uv = uv;

    return result;
}

float3 CalculateReflection(float3 normal, float3 ray)
{
    float3 x = cross(normal, ray);
    if (dot(x, x) <= 0) {
        return ray;
    }

    float3 z = cross(normal, x);

    x = normalize(x);
    z = normalize(z);

    float3x3 mat = {
        x[0], normal[0], z[0],
        x[1], normal[1], z[1],
        x[2], normal[2], z[2],
    };

    float3x3 trMat = transpose(mat);

    float3 refl = mul(trMat, ray);
    refl = float3(-refl[0], refl[1], -refl[2]);
    refl = mul(mat, refl);

    return refl;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 normal = p_normalTexture.Sample(p_sampler, float2(input.uv.x, 1 - input.uv.y)).xyz;
    if (dot(normal, normal) <= 0) {
        return float4(0.0, 0.2, 0.4, 1.0);
    }

    float3 diffuse = p_diffuseTexture.Sample(p_sampler, float2(input.uv.x, 1 - input.uv.y)).xyz;
    float3 position = p_positionTexture.Sample(p_sampler, float2(input.uv.x, 1 - input.uv.y)).xyz;

    float4 specular = p_specularTexture.Sample(p_sampler, float2(input.uv.x, 1 - input.uv.y));
    float3 specularColor = specular.xyz;

    float3 color = m_ambientIntensity * diffuse * m_ambientLight;

    for (int i = 0; i < m_numLights; ++i) {
        LightData cur = m_lights[i];
        float3 lightOffset = cur.m_position - position;
        float3 lightDir = normalize(lightOffset);

        float diffuseCoef = dot(normal, lightDir);

        if (diffuseCoef <= 0) {
            continue;
        }

        float3 viewOffset = m_camPos - position;
        float intensity = max(length(viewOffset), length(lightOffset)) / cur.m_intensity;
        intensity = 1 - clamp(intensity, 0, 1);

        color += intensity * diffuseCoef * diffuse * cur.m_color;

        float3 viewDir = normalize(viewOffset);
        float3 reflection = CalculateReflection(normal, lightDir);
        float specularCoef = dot(viewDir, reflection);
        if (specularCoef <= 0) {
            continue;
        }

        color += intensity * pow(specularCoef, specular.w) * specular * cur.m_color;
    }

    return float4(color, 1);
}
