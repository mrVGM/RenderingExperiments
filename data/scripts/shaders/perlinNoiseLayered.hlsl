cbuffer NoiseData : register(b0)
{
    float2 m_min;
    float2 m_max;

    float2 m_factor;
    float m_verticalOffset;

    float m_octaves;
    float m_persistance;
    float m_scale;
};

struct PSInput
{
    float4 position         : SV_POSITION;
    float2 uv               : UV;
};

PSInput VSMain(
    float2 position : POSITION)
{
    PSInput result;

    float x = position.x;
    float y = position.y;

    if (x < 0) {
        x = 2 * m_min.x - 1;
    }
    else {
        x = 2 * m_max.x - 1;
    }

    if (y < 0) {
        y = 2 * m_min.y - 1;
    }
    else {
        y = 2 * m_max.y - 1;
    }

    result.position = float4(x, y, 0, 1);
    result.uv = float2((position.x + 1) / 2, (position.y + 1) / 2);
    return result;
}

static int permutation[] = { 151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

int hash(int x)
{
    x %= 256;
    while (x < 0) {
        x += 256;
    }

    return permutation[x];
}

float smoothstep(float x)
{
    if (x <= 0) {
        return 0;
    }

    if (x >= 1) {
        return 1;
    }

    return 3 * x * x - 2 * x * x * x;
}

static float3 dirs[] = {
    float3(0,1,1),
    float3(0,1,-1),
    float3(0,-1,1),
    float3(0,-1,-1),
    float3(1,0,1),
    float3(1,0,-1),
    float3(-1,0,1),
    float3(-1,0,-1),
    float3(1,1,0),
    float3(1,-1,0),
    float3(-1,1,0),
    float3(-1,-1,0),
    float3(1,0,-1),
    float3(-1,0,-1),
    float3(0,-1,1),
    float3(0,1,1)
};

float3 noiseDir(int3 coord)
{
    int index = 0;
    index += hash(index + coord.x);
    index += hash(index + coord.y);
    index += hash(index + coord.z);

    index %= 12;

    return dirs[index];
}

float scalarNoise(float3 pos, int3 vertex)
{
    return dot(pos - vertex, noiseDir(vertex));
}

float calcNoise(float3 uvw)
{
    int3 cage_min = floor(uvw);
    float3 uvwNormalized = uvw - cage_min;

    float smoothX = smoothstep(uvwNormalized.x);
    float smoothY = smoothstep(uvwNormalized.y);
    float smoothZ = smoothstep(uvwNormalized.z);

    float downBackX = (1 - smoothX) * scalarNoise(uvw, cage_min + int3(0, 0, 0)) + smoothX * scalarNoise(uvw, cage_min + int3(1, 0, 0));
    float downFrontX = (1 - smoothX) * scalarNoise(uvw, cage_min + int3(0, 1, 0)) + smoothX * scalarNoise(uvw, cage_min + int3(1, 1, 0));

    float upBackX = (1 - smoothX) * scalarNoise(uvw, cage_min + int3(0, 0, 1)) + smoothX * scalarNoise(uvw, cage_min + int3(1, 0, 1));
    float upFrontX = (1 - smoothX) * scalarNoise(uvw, cage_min + int3(0, 1, 1)) + smoothX * scalarNoise(uvw, cage_min + int3(1, 1, 1));

    float down = (1 - smoothY) * downBackX + smoothY * downFrontX;
    float up = (1 - smoothY) * upBackX + smoothY * upFrontX;

    float h = (1 - smoothZ) * down + smoothZ * up;
    h += 0.5;

    return h;
}

float4 PSMain(float4 position : SV_POSITION, float2 uv : UV) : SV_Target
{
    uv *= m_factor;
    float3 uvw = float3(uv, m_verticalOffset);
    
    float scale = 1;
    float factor = m_persistance;
    float totalFactor = 0;

    float n = 0;

    for (int i = 0; i < m_octaves; ++i) {
        float curSample = calcNoise(scale * uvw + 4.3);
        n += factor * curSample;

        totalFactor += factor;
        scale *= m_scale;
        factor *= m_persistance;
    }

    n /= totalFactor;

    float3 color = 0.5 + 0.5 * cos(6.283185 * (3 * n + float3(0.15, 0, 0)));

    return float4(color, 1);
}

