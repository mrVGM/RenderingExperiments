struct SRVBuffElement
{
    float x;
    float y;
};

StructuredBuffer<SRVBuffElement> inData		: register(t0);    // SRV
RWTexture2D<float4> outData			        : register(u0);    // UAV

cbuffer InfoConstantBuff : register(b0)
{
    int m_texSize;
    int m_srvSize;

    float m_padding[62];
};

[numthreads(1, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	outData[DTid.xy] = float4(1,1,0,1);
}
