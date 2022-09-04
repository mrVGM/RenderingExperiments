struct Col
{
	float4 color;
};

StructuredBuffer<PosVelo>	inData	: register(t0);
RWStructuredBuffer<PosVelo>	outData	: register(u0);

[numthreads(1, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	outData[0] = float4(1,0,1,1);
}
