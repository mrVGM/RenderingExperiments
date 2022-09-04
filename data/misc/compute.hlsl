StructuredBuffer<float4> inData		: register(t0);    // SRV
RWTexture2D<float4> outData					: register(u0);    // UAV

[numthreads(1, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
}
