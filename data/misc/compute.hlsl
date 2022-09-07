struct SRVBuffElement
{
    float x;
    float y;
    float z;
};

StructuredBuffer<SRVBuffElement> inData		: register(t0);    // SRV
RWTexture3D<float4> outData			        : register(u0);    // UAV

cbuffer InfoConstantBuff : register(b0)
{
    int m_texSize;
    int m_srvSize;

    float m_padding[62];
};

int CoordToIndex(int3 coord)
{
    return coord.z * m_srvSize * m_srvSize + coord.y * m_srvSize + coord.x;
}

int3 IndexToCoord(int index)
{
    int col = index % m_srvSize;
    int row = index % m_srvSize;
    int plate = index / (m_srvSize * m_srvSize);

    return int3(col, row, plate);
}

void GetNeighbours(int3 coord, out float3 neighbours[27])
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
            cur.x = m_srvSize - 1;
        }
        if (cur.x >= m_srvSize) {
            cur.x = 0;
        }

        if (cur.y < 0) {
            cur.y = m_srvSize - 1;
        }
        if (cur.y >= m_srvSize) {
            cur.y = 0;
        }

        if (cur.z < 0) {
            cur.z = m_srvSize - 1;
        }
        if (cur.z >= m_srvSize) {
            cur.z = 0;
        }

        SRVBuffElement curPoint = inData[CoordToIndex(cur)];

        float3 curCoord = float3(curPoint.x, curPoint.y, curPoint.z);
        float3 localOffset = curCoord - cur;

        neighbours[i] = cube + localOffset;
    }
}

[numthreads(1, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
    float3 uvw = float3(DTid);
    uvw /= m_texSize;

    float3 gridCoord = uvw * m_srvSize;

    int3 squareCoord = floor(gridCoord);

    float3 neighbours[27];
    GetNeighbours(squareCoord, neighbours);

    float minDist = 4;
    for (int i = 0; i < 27; ++i) {
        float d = length(gridCoord - neighbours[i]);
        if (d < minDist) {
            minDist = d;
        }
    }

    float c = clamp(minDist, 0, 1);
    c = 1 - c;
	outData[DTid] = float4(c, c, c, 1);
}
