cbuffer buff : register(b0)
{
	matrix world;
	matrix mvp;
};

struct VS_INPUT {
	float4 pos : POSITION;
	float4 normal : NORMAL;
	float4 uv : UV;
};
struct VS_OUTPUT {
	float4 pos : SV_Position;
	float4 uv : UV;
	float4 normal : NORMAL;
	float4 worldPos : WORLDPOS;
};

void vs_main( VS_INPUT d, out VS_OUTPUT o )
{
	o.pos = mul(d.pos, mvp);
	o.uv = d.uv;
	o.worldPos = d.pos;

	float posZ = o.pos.z / o.pos.w;
	float depth = 1000 * 0.1 / (1000 - posZ * (1000 - 0.1));
	o.worldPos.w = depth;

	o.normal = d.normal;
}
