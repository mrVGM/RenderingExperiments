struct LightData
{
	float m_directional;
	float3 m_pos;
	float3 m_dir;
	float3 m_color;
	float m_intensity;
	float m_range;
};


Texture2D<float4> diffuseTex				: register(t0);
Texture2D<float4> specularTex				: register(t1);
Texture2D<float4> positionTex				: register(t2);
Texture2D<float4> normalTex					: register(t3);

StructuredBuffer<LightData> LightsBuf		: register(t4);

SamplerState samp				: register(s0);

cbuffer ambientLight : register(b0)
{
	float4 m_ambientLightColor		: packoffset(c0);
	float4 m_viewPos				: packoffset(c1);
	float m_ambientLightIntensity	: packoffset(c2);
	int m_lightsCount				: packoffset(c2.y);
};

float4 ps_main(float4 pos : SV_Position, float4 uv : UV) : SV_Target
{
	float3 normal = normalTex.Sample(samp, uv.xy).rgb;
	if (length(normal) <= 0) {
		clip(-1);
	}
	
	float3 sampleDiffuse	= diffuseTex.Sample(samp, uv.xy).rgb;
	float3 sampleSpecular	= specularTex.Sample(samp, uv.xy);
	float3 samplePosition	= positionTex.Sample(samp, uv.xy).rgb;
	float3 sampleNormal		= normalTex.Sample(samp, uv.xy).rgb;

	float specularExp		= specularTex.Sample(samp, uv.xy).a;

	float3 res;
	{
		// ambient light
		float3 color = m_ambientLightColor.xyz;
		res = m_ambientLightIntensity * color * sampleDiffuse;
	}
	
	{
		// diffuse light

		for (int i = 0; i < m_lightsCount; ++i) {
			LightData cur = LightsBuf[i];
			float intensity = cur.m_intensity;
			float3 dir = cur.m_dir;

			if (cur.m_directional < 1) {
				dir = samplePosition - cur.m_pos;
				intensity *= 1 - min(1, length(dir) / cur.m_range);
				dir = normalize(dir);
			}


			res += intensity * max(0, dot(sampleNormal, -dir)) * cur.m_color * sampleDiffuse;
		}
	}

	{
		// specular light

		for (int i = 0; i < m_lightsCount; ++i) {
			LightData cur = LightsBuf[i];
			if (cur.m_directional > 0) {
				continue;
			}

			float intensity = cur.m_intensity;
			float3 dir = samplePosition - cur.m_pos;
			float3 viewDir = m_viewPos - samplePosition;

			intensity *= 1 - min(1, (length(dir) + length(viewDir)) / cur.m_range);
			dir = normalize(dir);
			
			float3 refl = -dir;

			float3 p = cross(sampleNormal, dir);
			if (length(p) > 0.0001) {
				p = normalize(p);
				float3 z = cross(p, sampleNormal);

				float3x3 m = {
					sampleNormal.x, sampleNormal.y, sampleNormal.z,
					p.x, p.y, p.z,
					z.x, z.y, z.z,
				};
				float3x3 inv = transpose(m);
				refl = mul(refl, inv);
				refl.z = -refl.z;
				refl = mul(refl, m);
			}

			viewDir = normalize(viewDir);

			res += intensity * pow(max(0, dot(viewDir, refl)), specularExp) * cur.m_color * sampleSpecular;
		}
	}

	return float4(res, 1);
}
