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

cbuffer RaymarchSettings : register(b1)
{
    float4 m_sphere;
    float4 m_sphere2;

    float m_mandelbulbPower;
};

struct PSInput
{
    float4 position         : SV_POSITION;
    float2 uv               : UV;
    float3 world_position   : WORLD;
    float3 center           : CENTER;
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
    float power = m_mandelbulbPower;

    float3 z = pos;
    float dr = 1;
    float r;

    for (int i = 0; i < 30; ++i) {
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

float3 getSunPos()
{
    const float M_PI = 3.1415926535897932384626433832795;

    float sunAzimuth = M_PI * m_sunAzimuth / 180.0;
    float sunAltitude = M_PI * m_sunAltitude / 180.0;
    float3 sunDir = float3(cos(sunAzimuth) * sin(sunAltitude), cos(sunAltitude), sin(sunAzimuth) * sin(sunAltitude));

    float3 sunPos = m_camPos + 10000 * sunDir;

    return sunPos;
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
    result.center = objectPosition;

    return result;
}

float4 PSMain(
    float4 position : SV_POSITION,
    float2 uv : UV,
    float3 world_position : WORLD,
    float3 center : CENTER) : SV_Target
{
    float3 dir = normalize(world_position - m_camPos.xyz);

    float3 curPos = m_camPos;
    float t = sdf(world_position - center);

    for (int i = 0; i < 200; ++i) {
        curPos += t * dir;

        t = sdf(curPos - center);

        if (t < 0.001) {
            float3 normal = calcNormal(curPos - center);
            float3 sunDir = normalize(getSunPos() - curPos);
            float c = (1 + dot(sunDir, normal)) / 2;
            c = max(c, 0.2);

            float n = length(curPos);
            float3 col = c * (0.5 + 0.5 * cos(6.283185 * (3 * n + float3(0.15, 0, 0))));

            return float4(col, 1);
        }
    }

    return float4(0,0,0,0);
}

