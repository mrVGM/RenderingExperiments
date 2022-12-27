static const float PI = 3.14159265359f;

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

cbuffer CloudAreaSettings : register(b1)
{
    float4 m_lightColor;
    float4 m_darkColor;

    float3 m_lightPosition;
    float m_lightIntensity;

    float m_cloudAbsorbtion;
    float m_airLightAbsorbtion;
    float m_cloudLightAbsorbtion;
    float m_g;

    float m_sampleSteps;
    float m_sampleStepsTowardsLight;
    float m_randomOffsetStrength;
    float m_minDensity;
    float m_densityFactor;
    float m_densityOffset;
    float m_detailFactor;

    float m_shapeTexScale;
    float m_shapeTexOffset;
    float m_shapeOffsetSpeed;
    float m_shape1Weight;
    float m_shape2Weight;
    float m_shape3Weight;

    float m_detailTexScale;
    float m_detailTexOffset;
    float m_detailOffsetSpeed;
    float m_detail1Weight;
    float m_detail2Weight;
    float m_detail3Weight;
};

Texture3D p_shapeTexture    : register(t0);
Texture3D p_detailTexture   : register(t1);
SamplerState p_sampler  : register(s0);

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
int findIntersections(Wall walls[6], float3 startPoint, float3 dir, out float3 intersections[2])
{
    int num = 0;
    for (int i = 0; i < 6; ++i) {
        float3 tmp;
        bool res = intersectWall(walls[i], startPoint, dir, tmp);
        if (res) {
            intersections[num++] = tmp;
        }
        if (num == 2) {
            break;
        }
    }

    if (num == 2) {
        if (length(intersections[0] - startPoint) > length(intersections[1] - startPoint)) {
            float3 tmp = intersections[0];
            intersections[0] = intersections[1];
            intersections[1] = tmp;
        }
    }

    return num;
}



float randNoise(float3 value) {
    float3 smallValue = sin(value);
    float random = dot(smallValue, float3(12.9898, 78.233, 37.719));
    random = sin(random) * 143758.5453;
    random -= floor(random);
    return random;
}
float sampleShape(float3 coord)
{
    float3 newCoord = m_shapeTexScale * coord + float3(m_time * m_shapeOffsetSpeed + m_shapeTexOffset, 0, 0);
    newCoord = newCoord - floor(newCoord);
    float4 tex = p_shapeTexture.Sample(p_sampler, newCoord);
    float3 w = float3(m_shape1Weight, m_shape2Weight, m_shape3Weight);
    w /= dot(w, 1);
    float worly = dot(1 - tex.yzw, w);
    float res = tex.x * worly;
    if (res < m_minDensity) {
        res = 0;
    }
    return res;
}

float sampleDetail(float3 coord)
{
    float3 newCoord = m_detailTexScale * coord + float3(0, 0, m_time * m_detailOffsetSpeed + m_detailTexOffset);
    newCoord = newCoord - floor(newCoord);
    float4 tex = p_detailTexture.Sample(p_sampler, newCoord);
    float3 w = float3(m_detail1Weight, m_detail2Weight, m_detail3Weight);
    w /= dot(w, 1);
    float worly = dot(1 - tex.yzw, w);
    return worly;
}

float sampleCloud(float3 coord)
{
    float shape = sampleShape(coord);
    float detail = sampleDetail(coord);

    float maxShape = float3(m_shape1Weight, m_shape2Weight, m_shape3Weight);
    float erosion = shape * shape * shape / (maxShape * maxShape * maxShape);

    float res = shape * m_densityFactor - detail * m_detailFactor * erosion;
    res = max(0, res);
    return res;
}



float phase(float g, float cosAngle)
{
    float g2 = g * g;
    float x = 0.5 * (1 - g2) / pow(1 + g2 - 2 * g * cosAngle, 1.5);
    float y = 0.5 * (1 - g2) / pow(1 + g2 + 2 * g * cosAngle, 1.5);
    return 0.5 * (x + y);
}
float simpleScattering(float lightTransmittance, float angleCos)
{
    return phase(m_g, angleCos)
        * exp(-lightTransmittance * m_cloudLightAbsorbtion)
        * (1 - exp(-2 * lightTransmittance * m_cloudLightAbsorbtion));
}
float multipleOctaveScattering(float density, float angleCos)
{
    float attenuation = 0.2;
    float contribution = 0.4;
    float phaseAttenuation = 0.1;

    const float scatteringOctaves = 4.0;

    float a = 1.0;
    float b = 1.0;
    float c = 1.0;
    float g = m_g;

    float luminance = 0.0;

    for (float i = 0.0; i < scatteringOctaves; ++i) {
        float phaseFunction = phase(c * g, angleCos);
        float beers = exp(-density * m_cloudLightAbsorbtion * a) *
            (1 - exp(-2 * density * m_cloudLightAbsorbtion * a));

        luminance += b * phaseFunction * beers;

        a *= attenuation;
        b *= contribution;
        c *= (1 - phaseAttenuation);
    }

    return luminance;
}

float lightMarch(Wall walls[6], float3 pos, out float energy)
{
    float3 hits[2];
    int num = findIntersections(walls, pos, normalize(m_lightPosition - pos), hits);

    if (num != 1) {
        return 0;
    }

    float distToLight = length(hits[0] - m_lightPosition);
    energy = m_lightIntensity * exp(-distToLight * m_airLightAbsorbtion);

    float lightTransmittance = 0;

    {
        float cloudDist = length(hits[0] - pos);
        float stepSize = cloudDist / m_sampleStepsTowardsLight;

        float randOffset = randNoise(pos) * stepSize * m_randomOffsetStrength;
        pos += randOffset * normalize(hits[0] - pos);
        cloudDist -= randOffset;
        stepSize = cloudDist / m_sampleStepsTowardsLight;

        [unroll(10)]
        for (int i = 1; i <= m_sampleStepsTowardsLight; ++i) {
            float c = i / m_sampleStepsTowardsLight;
            float3 testPoint = (1 - c) * hits[0] + c * pos;

            float n = sampleCloud(testPoint) * stepSize;
            lightTransmittance += n;
        }
    }

    return lightTransmittance;
}

float2 cloudMarch(Wall walls[6], float3 hitPoint)
{
    float3 hits[2];
    int intersections = findIntersections(walls, m_camPos.xyz, normalize(hitPoint - m_camPos.xyz), hits);

    if (intersections != 2) {
        return float2(-1, -1);
    }

    float light = 0;
    float transmittance = -m_densityOffset;
    float lightTransmittance = -m_densityOffset;

    float cloudDist = length(hits[1] - hits[0]);
    float stepSize = cloudDist / m_sampleSteps;

    float randOffset = randNoise(hits[0]) * stepSize * m_randomOffsetStrength;
    hits[0] += randOffset * normalize(hits[1] - hits[0]);
    cloudDist -= randOffset;
    stepSize = cloudDist / m_sampleSteps;

    [unroll(50)]
    for (int i = 1; i <= m_sampleSteps; ++i) {
        float c = i / m_sampleSteps;
        float3 testPoint = (1 - c) * hits[0] + c * hits[1];

        float n = sampleCloud(testPoint) * stepSize;
        lightTransmittance += n;
        transmittance += n;

        float energy;
        float l = lightMarch(walls, testPoint, energy);

        float angleCos = dot(
            normalize(testPoint - m_lightPosition),
            normalize(m_camPos - testPoint)
        );

        //light += energy * simpleScattering(max(0, lightTransmittance + l), angleCos);
        light += energy * multipleOctaveScattering(max(0, lightTransmittance + l), angleCos);
    }
    return float2(light, exp(-transmittance * m_cloudAbsorbtion));
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
    float2 marchResult = cloudMarch(walls, world_position);
    float3 color = (1 - marchResult.x) * m_darkColor + marchResult.x * m_lightColor;
    return float4(color, 1 - marchResult.y); 
}
