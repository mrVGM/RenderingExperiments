struct PSInput
{
    float4 position : SV_POSITION;
};

PSInput VSMain(float2 position : POSITION)
{
    PSInput result;

    result.position = float4(position, 0, 1);

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(0, 1, 0, 1);
}
