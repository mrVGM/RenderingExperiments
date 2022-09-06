StructuredBuffer<float4> inData		: register(t0);    // SRV
RWTexture2D<float4> outData					: register(u0);    // UAV

cbuffer SceneConstantBuffer : register(b0)
{
    float4 color;
    float4 padding[15];
};

[numthreads(1, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	outData[DTid.xy] = color;
}
