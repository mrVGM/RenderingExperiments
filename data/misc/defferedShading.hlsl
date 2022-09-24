struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : UV;
};

Texture2D p_texture     : register(t0);
SamplerState p_sampler  : register(s0);

PSInput VSMain(float2 position : POSITION, float2 uv : UV)
{
    PSInput result;

    result.position = float4(0.5 * position, 0, 1);
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return p_texture.Sample(p_sampler, float2(input.uv.x, 1 - input.uv.y));
}
