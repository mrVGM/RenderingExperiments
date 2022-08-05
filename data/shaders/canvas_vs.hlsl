struct VS_INPUT {
	float4 pos : POSITION;
};
struct VS_OUTPUT {
	float4 pos : SV_Position;
	float4 uv : UV;
};

void vs_main(VS_INPUT d, out VS_OUTPUT o)
{
	o.pos = d.pos;
	float2 uv = o.pos * float2(1, -1) + float2(1, 1);
	uv = uv / 2;
	o.uv = float4(uv, 0, 1);
}
