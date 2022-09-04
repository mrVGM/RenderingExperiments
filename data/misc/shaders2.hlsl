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

cbuffer SceneConstantBuffer : register(b0)
{
    float2 centerPos;
    float2 scale;
    float4 coloe;

    float4 padding[14];
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
    float4 coloe : COLOE;
};

PSInput VSMain(float2 position : POSITION, float2 uv : UV)
{
    PSInput result;

    result.position = float4(position * scale + centerPos, 0, 1);
    result.uv = uv;

    result.coloe = coloe;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 offset = input.uv - float2(0.5, 0.5);
    float l = length(offset);
    if (l > 0.5) {
        return float4(0, 0, 0, 1);
    }
    return input.coloe;
}
