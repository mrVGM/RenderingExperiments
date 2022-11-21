struct PSInput
{
    float4 position         : SV_POSITION;
};

PSInput VSMain(
    float2 position : POSITION)
{
    PSInput result;
    result.position = float4(position.x, position.y, 0, 1);
    return result;
}

float4 PSMain(float4 position : SV_POSITION) : SV_Target
{
    return float4(1,1,0,1);
}
