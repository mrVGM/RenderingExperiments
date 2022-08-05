Texture3D<float4> tex		: register(t0);

SamplerState samp			: register(s0);

float4 ps_main(float4 pos : SV_Position, float4 uv : UV) : SV_Target
{
	float minX = 0.7;
	float minY = 0.7;

	if (uv.x < minX || uv.y < minY) {
		clip(-1);
	}
	
	float2 texCoord = (float2(uv.x, uv.y) - float2(minX, minY)) / float2(1 - minX, 1 - minY);
	float d = tex.Sample(samp, float3(texCoord.x, 0.5, texCoord.y)).r;

	return float4(d, d, d, 1);
}
