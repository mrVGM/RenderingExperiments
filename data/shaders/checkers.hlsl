cbuffer colorBuf : register(b0)
{
	float4 color1;
	float4 color2;
	float size;
};

struct PixelOut
{
	float4 diffuse		: SV_Target0;
	float4 specular		: SV_Target1;
	float4 position		: SV_Target2;
	float4 normal		: SV_Target3;
};

PixelOut ps_main(float4 pos : SV_Position, float4 uv : UV, float4 normal : NORMAL, float4 worldPos : WORLDPOS) : SV_Target
{
	float t = floor(size * uv.x) % 2;
	float p = floor(size * uv.y) % 2;

	t *= 2;
	t -= 1;
	p *= 2;
	p -= 1;

	PixelOut res;

	if (t * p > 0) {
		res.diffuse = color1;
	} else {
		res.diffuse = color2;
	}
	res.specular = float4(0.2, 0.2, 0.2, 20.0);
	res.position = worldPos;
	res.normal = normal;

	return res;
}
