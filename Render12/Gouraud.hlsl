#include "Render12.hlsli"

Texture2D TexDiffuse[4096] : register(t0);

struct SPoly
{
    float4 Pos : Position0;
    float3 Color : Color0;
    float2 TexCoord : TexCoord0;
    uint2 PolyFlags : BlendIndices0;
};

struct VSOut
{
    float4 Pos : SV_Position;
    float3 Color : Color0;
    float2 TexCoord : TexCoord0;
    uint2 PolyFlags : BlendIndices0;
};

VSOut VSMain(const SPoly Input)
{
    VSOut Output;
    Output.Pos = mul(Input.Pos, ProjectionMatrix);
    Output.Color = Input.Color;
    Output.TexCoord = Input.TexCoord;
    Output.PolyFlags = Input.PolyFlags;
    return Output;
}

float4 PSMain(const VSOut Input) : SV_Target
{
    
    float4 Color = float4(Input.Color, 1.0f);

    const float4 Diffuse = TexDiffuse[Input.PolyFlags.y].Sample(SamLinear, Input.TexCoord);
    
    if (Input.PolyFlags.x & PF_Masked || Input.PolyFlags.x & PF_Modulated)
    {
        clip(Diffuse.a - 0.9f);
    }
    Color *= Diffuse;
    
    return Color;
}
