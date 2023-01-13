cbuffer MatrixBuffer : register(b0)
{
	float4x4 m_matrix;
	float4 m_camPos;
	float4 m_camRight;
	float4 m_camFwd;
	float4 m_camUp;

	float m_camFov;
	float m_aspect;
	float m_time;
	float m_airAbsorbtion;

	float m_sunAzimuth;
	float m_sunAltitude;
};

cbuffer MandelbulbData : register(b1)
{
	float m_mandelbulbPower;
};

static const float M_PI = 3.1415926535897932384626433832795;

struct PSInput
{
	float4 position         : SV_POSITION;
	float3 world_position   : WORLD;
};


float mandelbulbSDF(float3 pos)
{
	float power = m_mandelbulbPower;
	power = 8;

	float3 z = pos;
	float dr = 1;
	float r;

	for (int i = 0; i < 30; ++i) {
		r = length(z);
		if (r > 2) {
			break;
		}

		float theta = acos(z.z / r) * power;
		float phi = atan2(z.y, z.x) * power;
		float zr = pow(r, power);

		dr = pow(r, power - 1) * power * dr + 1;

		z = zr * float3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
		z += pos;
	}

	return 0.5 * log(r) * r / dr;
}

float sdf(float3 pos)
{
	return mandelbulbSDF(pos);
}

float3 calcNormal(float3 pos)
{
	float eps = 0.001;

	float3 s1 = float3(
		sdf(pos + float3(eps, 0, 0)),
		sdf(pos + float3(0, eps, 0)),
		sdf(pos + float3(0, 0, eps))
		);

	float3 s2 = float3(
		sdf(pos - float3(eps, 0, 0)),
		sdf(pos - float3(0, eps, 0)),
		sdf(pos - float3(0, 0, eps))
		);

	return normalize(s1 - s2);
}

float3 getSunPos()
{
	float sunAzimuth = M_PI * m_sunAzimuth / 180.0;
	float sunAltitude = M_PI * m_sunAltitude / 180.0;
	float3 sunDir = float3(cos(sunAzimuth) * sin(sunAltitude), cos(sunAltitude), sin(sunAzimuth) * sin(sunAltitude));

	float3 sunPos = m_camPos + 10000 * sunDir;

	return sunPos;
}

PSInput VSMain(
	float2 position : POSITION,
	float3 objectPosition : OBJECT_POSITION,
	float4 objectRotation : OBJECT_ROTATION,
	float3 objectScale : OBJECT_SCALE)
{
	PSInput output;

	float3 pos = objectScale * float3(position, 0);
	pos += objectPosition;

	output.position = float4(pos, 1);

	pos = m_camPos + m_camFwd;
	float fov = m_camFov / 2;
	fov = M_PI * fov / 180;

	float verticalScale = tan(fov);
	float horizontalScale = m_aspect * tan(fov);

	pos += output.position.x * horizontalScale * m_camRight.xyz + output.position.y * verticalScale * m_camUp.xyz;
	output.world_position = float4(pos, 1);

	return output;
}

float4 PSMain(
	float4 position : SV_POSITION,
	float3 world_position : WORLD) : SV_TARGET
{
	float3 dir = normalize(world_position - m_camPos.xyz);

	float3 curPos = m_camPos;
	float t = sdf(world_position);

	for (int i = 0; i < 200; ++i) {
		curPos += t * dir;

		t = sdf(curPos);

		if (t < 0.001) {
			float3 normal = calcNormal(curPos);
			float3 sunDir = normalize(getSunPos() - curPos);
			float c = (1 + dot(sunDir, normal)) / 2;
			c = max(c, 0.2);

			float n = length(curPos);
			float3 col = c * (0.5 + 0.5 * cos(6.283185 * (3 * n + float3(0.15, 0, 0))));

			return float4(col, 1);
		}
	}

	return float4(0,0,0,0);
}