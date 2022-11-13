struct SRVBuffElement
{
    float x;
    float y;
    float z;
};

RWTexture3D<float4> texData			        : register(u0);    // UAV

StructuredBuffer<SRVBuffElement> points1	: register(t0);    // SRV
StructuredBuffer<SRVBuffElement> points2	: register(t1);    // SRV
StructuredBuffer<SRVBuffElement> points3	: register(t2);    // SRV

cbuffer InfoConstantBuff : register(b0)
{
    int m_texSize;

    int m_cells1;
    int m_octaves1;
    float m_persistance1;
    float m_scale1;

    int m_cells2;
    int m_octaves2;
    float m_persistance2;
    float m_scale2;

    int m_cells3;
    int m_octaves3;
    float m_persistance3;
    float m_scale3;
};

struct FBMSettings
{
    int m_octaves;
    float m_persistance;
    float m_scale;
};

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

void GetNeighbours(int3 coord, StructuredBuffer<SRVBuffElement> buff, int size, out float3 neighbours[27])
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

        SRVBuffElement curPoint = buff[CoordToIndex(cur, size)];

        float3 curCoord = float3(curPoint.x, curPoint.y, curPoint.z);
        float3 localOffset = curCoord - cur;

        neighbours[i] = cube + localOffset;
    }
}

float SampleWorlyNoise(float3 uvw, float uvwScale, StructuredBuffer<SRVBuffElement> buff, int size)
{
    uvw /= m_texSize;
    uvw *= uvwScale;
    uvw -= floor(uvw);

    float3 gridCoord = uvw * size;
    int3 squareCoord = floor(gridCoord);

    float3 neighbours[27];
    GetNeighbours(squareCoord, buff, size, neighbours);

    float minDist = 4;
    for (int i = 0; i < 27; ++i) {
        float d = length(gridCoord - neighbours[i]);
        if (d < minDist) {
            minDist = d;
        }
    }

    return clamp(minDist, 0, 1);
}

float SampleFBM(float3 uvw, FBMSettings fbm, StructuredBuffer<SRVBuffElement> buff, int size)
{
    float res = 0;

    float scale = 1;
    float factor = fbm.m_persistance;
    float totalFactor = 0;

    for (int i = 0; i < fbm.m_octaves; ++i) {
        float curSample = SampleWorlyNoise(uvw, scale, buff, size);
        res += factor * curSample;

        totalFactor += factor;
        scale *= fbm.m_scale;
        factor *= fbm.m_persistance;
    }

    return res / totalFactor;
}

[numthreads(16, 16, 4)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
    float3 uvw = float3(DTid);


    FBMSettings fbm;
    fbm.m_octaves = m_octaves1;
    fbm.m_persistance = m_persistance1;
    fbm.m_scale = m_scale1;
    float noise1Value = SampleFBM(uvw, fbm, points1, m_cells1);

    fbm.m_octaves = m_octaves2;
    fbm.m_persistance = m_persistance2;
    fbm.m_scale = m_scale2;
    float noise2Value = SampleFBM(uvw, fbm, points2, m_cells2);

    fbm.m_octaves = m_octaves3;
    fbm.m_persistance = m_persistance3;
    fbm.m_scale = m_scale3;
    float noise3Value = SampleFBM(uvw, fbm, points3, m_cells3);

    texData[DTid] = float4(noise1Value, noise2Value, noise3Value, 1);
    return;
}
