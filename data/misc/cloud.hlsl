cbuffer MVCMatrix : register(b0)
{
    float4x4 m_matrix;
    float3 m_camPos;
    float dummy;
    float3 m_sunPos;
};

cbuffer CloudSettings : register(b1)
{
    float cs_lighting;
    float cs_lightingFactor;

    float cs_lightColorR;
    float cs_lightColorG;
    float cs_lightColorB;

    float cs_darkColorR;
    float cs_darkColorG;
    float cs_darkColorB;

    float cs_MaxSampleSteps;
    float cs_StepSize;

    float cs_LightAbsorptionTowardSun;
    float cs_DarknessThreshold;
    float cs_LightAbsorptionThroughCloud;

    float cs_PhaseX;
    float cs_PhaseY;
    float cs_PhaseZ;
    float cs_PhaseW;

    float cs_WeightR;
    float cs_WeightG;
    float cs_WeightB;

    float cs_DensityOffset;

    float cs_DensityThresholdR;
    float cs_DensityMultiplierR;
    float cs_UVWFactorR;

    float cs_DensityThresholdG;
    float cs_DensityMultiplierG;
    float cs_UVWFactorG;

    float cs_DensityThresholdB;
    float cs_DensityMultiplierB;
    float cs_UVWFactorB;
};

cbuffer LightsInfo : register(b2)
{
    float m_numLights;
    float3 m_ambientLight;
    float m_ambientIntensity;
};

struct LightData
{
    float3 m_position;
    float3 m_color;
    float m_intensity;
};


Texture3D p_texture     : register(t0);
StructuredBuffer<LightData> m_lights : register(t1);

SamplerState p_sampler  : register(s0);

struct PSInput
{
    float4 position         : SV_POSITION;
    float4 world_position   : WORLD_POSITION;
    float2 uv               : UV;
};

float4 multiplyQuat(float4 q1, float4 q2)
{
    float a = q1.x * q2.x - q1.y * q2.y - q1.z * q2.z - q1.w * q2.w;
    float b = q1.x * q2.y + q1.y * q2.x + q1.z * q2.w - q1.w * q2.z;
    float c = q1.x * q2.z - q1.y * q2.w + q1.z * q2.x + q1.w * q2.y;
    float d = q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;

    return float4(a, b, c, d);
}

float4 conjugateQuat(float4 q)
{
    return float4(q.x, -q.y, -q.z, -q.w);
}

float3 rotateVector(float3 v, float4 rotation)
{
    float4 vQ = float4(0, v);
    float4 conjRot = conjugateQuat(rotation);
    float4 rotatedVQ = multiplyQuat(rotation, multiplyQuat(vQ, conjRot));

    return float3(rotatedVQ.y, rotatedVQ.z, rotatedVQ.w);
}
PSInput VSMain(
    float3 position : POSITION,
    float3 normal : NORMAL,
    float2 uv : UV,
    float3 objectPosition : OBJECT_POSITION,
    float4 objectRotation : OBJECT_ROTATION,
    float3 objectScale : OBJECT_SCALE)
{
    PSInput result;
    float3 scaledPos = objectScale * position;
    float3 rotatedPos = rotateVector(scaledPos, objectRotation);
    float3 pos = objectPosition + rotatedPos;

    result.position = mul(m_matrix, float4(pos, 1));
    result.world_position = float4(pos, 1);
    result.uv = uv;
    return result;
}

float sampleDensityChannel(int channel, float3 pos)
{
    float densityThreshold = cs_DensityThresholdR;
    float densityMultiplier = cs_DensityMultiplierR;
    float uvwFactor = cs_UVWFactorR;

    if (channel == 1) {
        densityThreshold = cs_DensityThresholdG;
        densityMultiplier = cs_DensityMultiplierG;
        uvwFactor = cs_UVWFactorG;
    }

    if (channel == 2) {
        densityThreshold = cs_DensityThresholdB;
        densityMultiplier = cs_DensityMultiplierB;
        uvwFactor = cs_UVWFactorB;
    }

    float3 uvw = uvwFactor * pos;
    float4 textureColor = p_texture.Sample(p_sampler, uvw);
    float textureVal = textureColor.r;
    if (channel == 1) {
        textureVal = textureColor.g;
    }
    if (channel == 2) {
        textureVal = textureColor.b;
    }

    float density = max(0, (1 - textureVal) - densityThreshold);
    density *= densityMultiplier;
    return density;
}

float sampleDensity(float3 pos)
{
    float densityR = sampleDensityChannel(0, pos);
    float densityG = sampleDensityChannel(1, pos);
    float densityB = sampleDensityChannel(2, pos);

    float density = cs_WeightR * densityR + cs_WeightG * densityG + cs_WeightB * densityB;
    return density;
}

float hg(float a, float g)
{
    float g2 = g * g;
    return (1 - g2) / (4 * 3.1415 * pow(1 + g2 - 2 * g * (a), 1.5));
}

float phase(float a)
{
    float blend = 0.5;
    float hgBlend = hg(a, cs_PhaseX) * (1 - blend) + hg(a, -cs_PhaseY) * blend;
    return cs_PhaseZ + hgBlend * cs_PhaseW;
}

float lightTransmittance(float3 pos)
{
    float stepSize = cs_StepSize;
    float3 sunDir = normalize(m_sunPos);

    float transmittance = 0;
    for (int i = 0; i < 6; ++i) {
        float3 curPoint = pos + i * sunDir * stepSize;
        float density = 0.7 * (sampleDensityChannel(0, curPoint) - 0.3 * sampleDensityChannel(1, curPoint));
        transmittance += density * stepSize;
    }

    return transmittance;
}

float4 PSMain(
    float4 position : SV_POSITION,
    float4 world_position : WORLD_POSITION,
    float2 uv : UV) : SV_TARGET
{
    /*
    float grad = p_texture.Sample(p_sampler, float3(uv, 0));
    return float4(grad, grad, grad, 1);
    */

    const float stepSize = cs_StepSize;
    float transmittance = 0;
    float transmittanceG = 0;
    float3 viewDir = normalize(world_position.xyz - m_camPos);

    float lightEnergy = 0;

    for (int i = 1; i <= 64; ++i) {
        float3 curPoint = world_position + i * viewDir * stepSize;
        float density = 0.7 * (sampleDensityChannel(0, curPoint) - 0.3 * sampleDensityChannel(1, curPoint));
        transmittance += density * stepSize;

        float lightTr = max(0, cs_lightingFactor * lightTransmittance(curPoint));

        float tr = max(0, transmittance + cs_DensityOffset);

        lightEnergy += (1 - exp(-tr)) * exp(-lightTr) * (1 - exp(-2 * lightTr));
    }

    transmittance += cs_DensityOffset;
    transmittance = max(0, transmittance);

    float mainShape = 1 - exp(-transmittance);

    float3 lightColor = float3(cs_lightColorR, cs_lightColorG, cs_lightColorB);
    float3 darkColor = float3(cs_darkColorR, cs_darkColorG, cs_darkColorB);

    if (cs_lighting <= 0) {
        lightEnergy = 1;
    }

    lightEnergy = clamp(lightEnergy, 0, 1);

    return float4(lightEnergy * lightColor + (1 - lightEnergy) * darkColor, mainShape);
}
