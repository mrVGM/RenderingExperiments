cbuffer CamInfo : register(b0)
{
    float4 m_camPos;
    float4 m_bottomLeft;
    float4 m_bottomRight;
    float4 m_upLeft;

    float4 m_sun;

    float4 m_sphere;
    float4 m_sphere2;
};

struct PSInput
{
    float4 position         : SV_POSITION;
    float2 uv               : UV;
};

float smoothMax(float a, float b, float k)
{
    return log(exp(k * a) + exp(k * b)) / k;
}

float smoothMin(float a, float b, float k)
{
    return -smoothMax(-a, -b, k);
}

float mandelbulbSDF(float3 pos)
{
    float power = 8;

    float3 z = pos;
    float dr = 1;
    float r;

    for (int i = 0; i < 15; ++i) {
        r = length(z);
        if (r > 2) {
            break;
        }

        float theta = acos(z.z / r) * power;
        float phi = atan2(z.y, z.x) * power;
        float zr = pow(r, power);

        dr = pow(r, power - 1) * power * dr + 1;

        z = zr * float3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
        z += pos;
    }

    return 0.5 * log(r) * r / dr;
}

float sdf(float3 pos)
{
    return mandelbulbSDF(pos);

    float s1Dist = length(pos - m_sphere.xyz) - m_sphere.w;
    float s2Dist = length(pos - m_sphere2.xyz) - m_sphere2.w;

    return smoothMin(s1Dist, s2Dist, 10);
}

float3 calcNormal(float3 pos)
{
    float eps = 0.001;

    float3 s1 = float3(
        sdf(pos + float3(eps, 0, 0)),
        sdf(pos + float3(0, eps, 0)),
        sdf(pos + float3(0, 0, eps))
    );

    float3 s2 = float3(
        sdf(pos - float3(eps, 0, 0)),
        sdf(pos - float3(0, eps, 0)),
        sdf(pos - float3(0, 0, eps))
    );

    return normalize(s1 - s2);
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

    for (int i = 0; i < 200; ++i) {
        curPos += t * dir;

        t = sdf(curPos);

        if (t < 0.001) {
            float3 normal = calcNormal(curPos);
            float3 sunDir = normalize(m_sun.xyz - curPos);
            float c = (1 + dot(sunDir, normal)) / 2;
            c = max(c, 0.2);

            float3 col = c * float3(1, 1, 0);

            return float4(col, 1);
        }
    }

    return float4(0,0,0,1);
}

