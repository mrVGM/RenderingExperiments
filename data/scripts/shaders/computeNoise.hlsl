RWTexture3D<float4> texData			        : register(u0);    // UAV
StructuredBuffer<float3> points	            : register(t0);    // SRV

cbuffer InfoConstantBuff : register(b0)
{
    float m_texSize;

    float m_factor;

    float m_octaves;
    float m_persistance;
    float m_scale;

    float m_w1Size;
    float m_w2Size;
    float m_w3Size;
};


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
    float3 localDir = pos - vertex;
    int factor = min(m_factor, 256);
    vertex %= factor;
    return dot(localDir, noiseDir(vertex));
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



int CoordToIndex(int3 coord, int size)
{
    return coord.z * size * size + coord.y * size + coord.x;
}

int3 IndexToCoord(int index, int size)
{
    int col = index % size;
    int row = index % size;
    int plate = index / (size * size);

    return int3(col, row, plate);
}

void GetNeighbours(int3 coord, int size, int offset, out float3 neighbours[27])
{
    int3 offsets[27];

    int index = 0;
    for (int k = -1; k < 2; ++k) {
        for (int i = -1; i < 2; ++i) {
            for (int j = -1; j < 2; ++j) {
                offsets[index] = int3(j, i, k);
                index = index + 1;
            }
        }
    }

    for (int i = 0; i < 27; ++i) {
        int3 cur = coord + offsets[i];
        int3 cube = cur;


        if (cur.x < 0) {
            cur.x = size - 1;
        }
        if (cur.x >= size) {
            cur.x = 0;
        }

        if (cur.y < 0) {
            cur.y = size - 1;
        }
        if (cur.y >= size) {
            cur.y = 0;
        }

        if (cur.z < 0) {
            cur.z = size - 1;
        }
        if (cur.z >= size) {
            cur.z = 0;
        }

        float3 curPoint = points[CoordToIndex(cur, size) + offset];

        float3 curCoord = float3(curPoint.x, curPoint.y, curPoint.z);
        float3 localOffset = curCoord - cur;

        neighbours[i] = cube + localOffset;
    }
}

float sampleWorlyNoise(float3 uvw, int size, int offset)
{
    float3 gridCoord = uvw * size;
    int3 squareCoord = floor(gridCoord);

    float3 neighbours[27];
    GetNeighbours(squareCoord, size, offset, neighbours);

    float minDist = 4;
    for (int i = 0; i < 27; ++i) {
        float d = length(gridCoord - neighbours[i]);
        if (d < minDist) {
            minDist = d;
        }
    }

    return clamp(minDist, 0, 1);
}

float samplePerlinNoise(float3 uvw)
{
    uvw *= m_factor;

    float scale = 1;
    float factor = m_persistance;
    float totalFactor = 0;

    float n = 0;

    for (int i = 0; i < m_octaves; ++i) {
        float curSample = calcNoise(scale * uvw);
        n += factor * curSample;

        totalFactor += factor;
        scale *= m_scale;
        factor *= m_persistance;
    }
    n /= totalFactor;

    return n;
}

[numthreads(16, 16, 4)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
    float3 uvw = float3(DTid);
    uvw /= m_texSize;
    
    float pn = samplePerlinNoise(uvw);
    int offset = 0;
    float w1n = sampleWorlyNoise(uvw, m_w1Size, offset);
    offset += m_w1Size * m_w1Size * m_w1Size;
    
    float w2n = sampleWorlyNoise(uvw, m_w2Size, offset);
    offset += m_w2Size * m_w2Size * m_w2Size;

    float w3n = sampleWorlyNoise(uvw, m_w3Size, offset);

    texData[DTid] = float4(pn, w1n, w2n, w3n);
    return;
}
