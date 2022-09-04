StructuredBuffer<float4> inData		: register(t0);    // SRV
RWStructuredBuffer<float4> outData	: register(u0);    // UAV

[numthreads(1, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	outData[0] = float4(0,0,0.5,0.5);
	outData[1] = float4(1,1,0,1);
}
