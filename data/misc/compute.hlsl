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

    int m_cells11;
    int m_cells12;
    float m_blend1;

    int m_cells21;
    int m_cells22;
    float m_blend2;

    int m_cells31;
    int m_cells32;
    float m_blend3;
};

int CoordToIndex(int3 coord, int size, int offset)
{
    return offset + coord.z * size * size + coord.y * size + coord.x;
}

int3 IndexToCoord(int index, int size, int offset)
{
    index -= offset;

    int col = index % size;
    int row = index % size;
    int plate = index / (size * size);

    return int3(col, row, plate);
}

void GetNeighbours(int3 coord, StructuredBuffer<SRVBuffElement> buff, int size, int offset, out float3 neighbours[27])
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

        SRVBuffElement curPoint = buff[CoordToIndex(cur, size, offset)];

        float3 curCoord = float3(curPoint.x, curPoint.y, curPoint.z);
        float3 localOffset = curCoord - cur;

        neighbours[i] = cube + localOffset;
    }
}

float SampleWorlyNoise(float3 uvw, StructuredBuffer<SRVBuffElement> buff, int size, int offset)
{
    uvw /= m_texSize;
    float3 gridCoord = uvw * size;
    int3 squareCoord = floor(gridCoord);

    float3 neighbours[27];
    GetNeighbours(squareCoord, buff, size, offset, neighbours);

    float minDist = 4;
    for (int i = 0; i < 27; ++i) {
        float d = length(gridCoord - neighbours[i]);
        if (d < minDist) {
            minDist = d;
        }
    }

    return clamp(minDist, 0, 1);
}

[numthreads(16, 16, 4)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
    float3 uvw = float3(DTid);

    float noise1Value1 = SampleWorlyNoise(uvw, points1, m_cells11, 0);
    float noise1Value2 = SampleWorlyNoise(uvw, points1, m_cells12, m_cells11 * m_cells11 * m_cells11);
    float noise1Value = m_blend1 * noise1Value1 + (1 - m_blend1) * noise1Value2;

    float noise2Value1 = SampleWorlyNoise(uvw, points2, m_cells21, 0);
    float noise2Value2 = SampleWorlyNoise(uvw, points2, m_cells22, m_cells21 * m_cells21 * m_cells21);
    float noise2Value = m_blend2 * noise2Value1 + (1 - m_blend2) * noise2Value2;

    float noise3Value1 = SampleWorlyNoise(uvw, points3, m_cells31, 0);
    float noise3Value2 = SampleWorlyNoise(uvw, points3, m_cells32, m_cells31 * m_cells31 * m_cells31);
    float noise3Value = m_blend3 * noise3Value1 + (1 - m_blend3) * noise3Value2;

    texData[DTid] = float4(noise1Value, noise2Value, noise3Value, 1);
    return;
}
