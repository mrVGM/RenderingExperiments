cbuffer MVCMatrix : register(b0)
{
    float4x4 m_matrix;
    float3 m_camPos;
};

Texture3D p_texture     : register(t0);
SamplerState p_sampler  : register(s0);

struct CubeWall
{
    float3 m_origin;
    float3 m_right;
    float3 m_forward;
    float3 m_up;
    float3 m_axisScale;
};

struct PSInput
{
    float4 position         : SV_POSITION;
    float4 world_position   : WORLD_POSITION;

    CubeWall cube_walls[6]  : CUBE_WALLS;
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

void generateCubeWalls(
    float3 position,
    float4 rotation,
    float3 scale,
    out CubeWall walls[6]
)
{
    float3 xRotated = rotateVector(float3(1, 0, 0), rotation);
    float3 yRotated = rotateVector(float3(0, 1, 0), rotation);
    float3 zRotated = rotateVector(float3(0, 0, 1), rotation);

    walls[0].m_origin = position + scale.x * xRotated;
    walls[0].m_right = zRotated;
    walls[0].m_forward = yRotated;
    walls[0].m_up = xRotated;
    walls[0].m_axisScale = float3(scale.z, scale.y, scale.x);

    walls[1].m_origin = position - scale.x * xRotated;
    walls[1].m_right = -zRotated;
    walls[1].m_forward = yRotated;
    walls[1].m_up = -xRotated;
    walls[1].m_axisScale = float3(scale.z, scale.y, scale.x);



    walls[2].m_origin = position + scale.z * zRotated;
    walls[2].m_right = -xRotated;
    walls[2].m_forward = yRotated;
    walls[2].m_up = zRotated;
    walls[2].m_axisScale = float3(scale.x, scale.y, scale.z);

    walls[3].m_origin = position - scale.z * zRotated;
    walls[3].m_right = xRotated;
    walls[3].m_forward = yRotated;
    walls[3].m_up = -zRotated;
    walls[3].m_axisScale = float3(scale.x, scale.y, scale.z);



    walls[4].m_origin = position + scale.y * yRotated;
    walls[4].m_right = xRotated;
    walls[4].m_forward = zRotated;
    walls[4].m_up = yRotated;
    walls[4].m_axisScale = float3(scale.x, scale.z, scale.y);

    walls[5].m_origin = position - scale.y * yRotated;
    walls[5].m_right = xRotated;
    walls[5].m_forward = -zRotated;
    walls[5].m_up = -yRotated;
    walls[5].m_axisScale = float3(scale.x, scale.z, scale.y);
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

    generateCubeWalls(objectPosition, objectRotation, objectScale, result.cube_walls);

    return result;
}

bool intersectWall(float3 rayOrigin, float3 rayDir, CubeWall wall, out float3 position)
{
    float3x3 mat = float3x3(wall.m_right, wall.m_forward, wall.m_up);
    float3x3 invMat = transpose(mat);

    float3 origin = rayOrigin - wall.m_origin;
    origin = mul(mat, origin);
    float3 dir = mul(mat, rayDir);

    if (origin.z * dir.z >= 0) {
        return false;
    }

    float coef = -origin.z / dir.z;
    float3 wallPos = origin + coef * dir;

    if (abs(wallPos.x) > wall.m_axisScale.x) {
        return false;
    }

    if (abs(wallPos.y) > wall.m_axisScale.y) {
        return false;
    }

    position = mul(invMat, wallPos) + wall.m_origin;

    return true;
}

float sampleDensity(float3 pos)
{
    float densityThreshold = 0.2;
    float densityMultiplier = 0.3;
    float3 uvw = 0.1 * pos;
    float density = max(0, p_texture.Sample(p_sampler, uvw).r - densityThreshold);
    density *= densityMultiplier;
    return density;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 offset = input.world_position - m_camPos;
    offset = normalize(offset);

    int index = 0;
    float3 hits[2];

    for (int i = 0; i < 6; ++i) {
        float3 pos;
        if (intersectWall(m_camPos, offset, input.cube_walls[i], pos)) {
            hits[index] = pos;
            index = index + 1;
        }
        if (index == 2) {
            break;
        }
    }

    if (index < 2) {
        return float4(1, 0, 0, 1);
    }
    if (index > 2) {
        return float4(0, 1, 0, 1);
    }

    float grad = length(hits[0] - hits[1]);
    grad /= 2;

    float coefs[2];
    coefs[0] = dot(hits[0] - m_camPos, offset);
    coefs[1] = dot(hits[1] - m_camPos, offset);

    float minCoef = min(coefs[0], coefs[1]);
    float maxCoef = max(coefs[0], coefs[1]);

    float step = 0.1;
    int maxSteps = 10;

    float density = 0;

    int stepsMade = 0;
    float curCoef = minCoef;
    while (curCoef <= maxCoef && stepsMade < maxSteps) {
        float3 curTestPoint = curCoef * offset + m_camPos;
        float curDensity = sampleDensity(curTestPoint);
        density += curDensity;

        curCoef += step;
        stepsMade += 1;
    }

    float transmittance = exp(-density);
    return float4(1, 1, 1, (1-transmittance));
}
