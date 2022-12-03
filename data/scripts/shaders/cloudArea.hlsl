static const float PI = 3.14159265359f;

cbuffer MVCMatrix : register(b0)
{
    float4x4 m_matrix;
    float4 m_camPos;
    float m_time;
    float m_airAbsorbtion;
};

cbuffer CloudAreaSettings : register(b1)
{
    float m_sampleSteps;
    float m_cloudAbsorbtion;
};

struct Wall
{
    float3 m_origin;
    float3 m_x;
    float3 m_y;
    float3 m_z;
};

struct PSInput
{
    float4 position         : SV_POSITION;
    float4 world_position   : WORLD_POSITION;
    float2 uv               : UV;
    Wall walls[6]           : WALLS;
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

void generateWalls(out Wall walls[6], float3 center, float4 rotation, float3 extents)
{
    walls[0].m_origin = extents.x * float3(1, 0, 0);
    walls[0].m_x = extents.z * float3(0, 0, 1);
    walls[0].m_y = extents.y * float3(0, 1, 0);
    walls[0].m_z = extents.x * float3(-1, 0, 0);

    walls[1].m_origin = -extents.x * float3(1, 0, 0);
    walls[1].m_x = extents.z * float3(0, 0, -1);
    walls[1].m_y = extents.y * float3(0, 1, 0);
    walls[1].m_z = extents.x * float3(1, 0, 0);


    walls[2].m_origin = extents.z * float3(0, 0, 1);
    walls[2].m_x = extents.x * float3(-1, 0, 0);
    walls[2].m_y = extents.y * float3(0, 1, 0);
    walls[2].m_z = extents.z * float3(0, 0, 1);

    walls[3].m_origin = -extents.z * float3(0, 0, 1);
    walls[3].m_x = extents.x * float3(1, 0, 0);
    walls[3].m_y = extents.y * float3(0, 1, 0);
    walls[3].m_z = extents.z * float3(0, 0, -1);



    walls[4].m_origin = extents.y * float3(0, 1, 0);
    walls[4].m_x = extents.x * float3(1, 0, 0);
    walls[4].m_y = extents.z * float3(0, 0, 1);
    walls[4].m_z = extents.y * float3(0, 1, 0);

    walls[5].m_origin = -extents.y * float3(0, 1, 0);
    walls[5].m_x = extents.x * float3(1, 0, 0);
    walls[5].m_y = extents.z * float3(0, 0, -1);
    walls[5].m_z = extents.y * float3(0, -1, 0);

    for (int i = 0; i < 6; ++i) {
        walls[i].m_origin = rotateVector(walls[i].m_origin, rotation);
        walls[i].m_x = rotateVector(walls[i].m_x, rotation);
        walls[i].m_y = rotateVector(walls[i].m_y, rotation);
        walls[i].m_z = rotateVector(walls[i].m_z, rotation);

        walls[i].m_origin += center;
    }
}

bool intersectWall(Wall wall, float3 startPoint, float3 dir, out float3 intersection)
{
    matrix mat = matrix(float4(normalize(wall.m_x), 0), float4(normalize(wall.m_y), 0), float4(normalize(wall.m_z), 0), float4(0, 0, 0, 1));
    
    float3 dirTr = mul(mat, float4(dir, 0)).xyz;
    float3 startPointTr = mul(mat, float4(startPoint - wall.m_origin, 1)).xyz;

    if (startPointTr.z * dirTr.z >= 0) {
        return false;
    }

    float c = -startPointTr.z / dirTr.z;

    float3 intersectionPoint = startPointTr + c * dirTr;

    if (abs(intersectionPoint.x) > length(wall.m_x)) {
        return false;
    }

    if (abs(intersectionPoint.y) > length(wall.m_y)) {
        return false;
    }

    intersection = startPoint + c * dir;
    return true;
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

    Wall walls[6];
    generateWalls(walls, objectPosition, objectRotation, objectScale);

    result.walls = walls;

    return result;
}

float4 PSMain(
    float4 position : SV_POSITION,
    float4 world_position : WORLD_POSITION,
    float2 uv : UV,
    Wall walls[6] : WALLS) : SV_TARGET
{
    int intersections = 0;

    float3 colors[6];
    colors[0] = float3(0.5, 1, 0);
    colors[1] = float3(0.5, 0, 1);
    colors[2] = float3(0, 0.5, 1);
    colors[3] = float3(1, 0.5, 0);
    colors[4] = float3(1, 0, 0.5);
    colors[5] = float3(0, 1, 0.5);

    float3 color = float3(0, 0, 0);

    for (int i = 0; i < 6; ++i) {
        Wall cur = walls[i];
        float3 hit;
        if (intersectWall(cur, m_camPos.xyz, normalize(world_position.xyz - m_camPos.xyz), hit)) {
            if (intersections == 0) {
                color = colors[i];
            }
            else {
                color = 0.5 * color + 0.5 * colors[i];
            }
            intersections += 1;
        }
    }

    return float4(color, 1);

    if (intersections == 2) {
        return float4(0, 1, 0, 1);
    }

    if (intersections == 0) {
        return float4(0, 0, 1, 1);
    }

    return float4(1, 0, 0, 1);
}
