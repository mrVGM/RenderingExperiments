struct SRVBuffElement
{
    float x;
    float y;
    float padding[2];
};

StructuredBuffer<SRVBuffElement> inData		: register(t0);    // SRV
RWTexture3D<float4> outData			        : register(u0);    // UAV

cbuffer InfoConstantBuff : register(b0)
{
    int m_texSize;
    int m_srvSize;

    float m_padding[62];
};

int CoordToIndex(int2 coord)
{
    return coord.y * m_srvSize + coord.x;
}

int2 IndexToCoord(int index)
{
    int col = index % m_srvSize;
    int row = index / m_srvSize;

    return int2(col, row);
}

void GetNeighbours(int2 coord, out float2 neighbours[9])
{
    int2 offsets[9] =
    {
        int2(0,     0),
        int2(0,    -1),
        int2(0,     1),
        int2(-1,    0),
        int2(-1,   -1),
        int2(-1,    1),
        int2(1,     0),
        int2(1,    -1),
        int2(1,     1)
    };

    for (int i = 0; i < 9; ++i) {
        int2 cur = coord + offsets[i];
        int2 square = cur;


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

        SRVBuffElement curPoint = inData[CoordToIndex(cur)];

        float2 curCoord = float2(curPoint.x, curPoint.y);
        float2 localOffset = curCoord - cur;

        neighbours[i] = square + localOffset;
    }
}

[numthreads(1, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
    float2 uv = float2(DTid.xy);
    uv /= m_texSize;

    float2 gridCoord = uv * m_srvSize;

    int2 squareCoord = floor(gridCoord);

    float2 neighbours[9];
    GetNeighbours(squareCoord, neighbours);

    float minDist = 2;
    for (int i = 0; i < 9; ++i) {
        float d = length(gridCoord - neighbours[i]);
        if (d < minDist) {
            minDist = d;
        }
    }

    float c = clamp(minDist, 0, 1);
    c = 1 - c;
	outData[DTid] = float4(c, c, c, 1);
}
