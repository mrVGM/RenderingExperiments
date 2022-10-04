cbuffer MVCMatrix : register(b0)
{
    float4x4 m_matrix;
    float m_padding[48];
};

struct PSInput
{
    float4 position         : SV_POSITION;
    float4 world_position   : WORLD_POSITION;
    float4 normal           : NORMAL;
    float2 uv               : UV;
};

struct PixelOutput
{
    float4 diffuse		: SV_Target0;
    float4 normal		: SV_Target1;
    float4 position		: SV_Target2;
    float4 specular		: SV_Target3;
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

    float4 posQ = float4(0, scaledPos);
    float4 conjRot = conjugateQuat(objectRotation);

    float4 rotatedPos = multiplyQuat(objectRotation, multiplyQuat(posQ, conjRot));

    float3 pos = float3(rotatedPos.y, rotatedPos.z, rotatedPos.w);
    result.position = mul(m_matrix, float4(objectPosition + pos, 1));
    result.world_position = float4(position, 1);
    result.normal = float4(normal, 1);
    result.uv = uv;

    return result;
}

PixelOutput PSMain(float4 position : SV_POSITION, float4 worldPosition : WORLD_POSITION, float4 normal : NORMAL)
{
    PixelOutput res;
    res.diffuse = float4(1, 0, 1, 1);
    res.normal = normal;
    res.position = worldPosition;
    res.specular = float4(0, 1, 0, 5);
    return res;
}
