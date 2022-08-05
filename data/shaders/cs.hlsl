struct Point
{
    float3 vec;
};

StructuredBuffer<Point> Points      : register(t0);

RWStructuredBuffer<Point> Max       : register(u0);
RWTexture3D<float4> tex             : register(u1);

cbuffer WorlyNoiseData : register(b0)
{
    int m_cells                     : packoffset(c0);
    int m_texResolution             : packoffset(c0.y);
};

[numthreads(1, 1, 1)]
void cs_main(uint3 DTid : SV_DispatchThreadID)
{
    float3 pos = float3(DTid.x, DTid.y, DTid.z);
    int3 cube = int3(DTid.x, DTid.y, DTid.z);

    pos /= m_cells * m_texResolution;
    cube /= m_cells * m_texResolution;

    int index = cube.y * m_cells * m_cells + cube.x * m_cells + cube.z;

    float l = length(Points[index].vec - pos);

    if (Max[0].vec.x < l) {
        Max[0].vec.x = l;
    }
    tex[DTid.xyz] = float4(l, 0, 0, 0);
}
