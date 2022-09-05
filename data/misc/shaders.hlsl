//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

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
    if (input.uv.x < 0.5) {
        return p_texture.Sample(p_sampler, input.uv);
    }
    return float4(input.uv, 0, 1);
}
