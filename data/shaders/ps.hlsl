cbuffer colorBuf : register(b0)
{
	float4 constantColor;
};

float4 ps_main( float4 pos : SV_Position) : SV_Target
{
	return float4(constantColor.xyz,1);
}
