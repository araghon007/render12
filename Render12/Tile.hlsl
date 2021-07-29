#include "Render12.hlsli"

Texture2D TexDiffuse[1024] : register(t0);

struct STile
{
    float4 XYPos : Position0; //Left, right, top, bottom in pixel coordinates
    float4 TexCoord : TexCoord0; //Left, right, top, bottom
    float3 Color : TexCoord1;
    uint2 PolyFlags : BlendIndices0; //PolyFlags, tex index
};

struct VSOut
{
    float4 Pos : SV_Position;
    float2 TexCoord : TexCoord0;
    float3 Color : TexCoord1;
    uint2 PolyFlags : BlendIndices0;
};

VSOut VSMain(const STile Tile, const uint VertexID : SV_VertexID)
{
    VSOut Output;
    const uint IndexX = VertexID / 2;
    const uint IndexY = 3 - VertexID % 2;
    Output.Pos = float4(-1.0f + 2.0f * (Tile.XYPos[IndexX] * fRes.z), 1.0f - 2.0f * (Tile.XYPos[IndexY] * fRes.w), 1.0f, 1.0f);
    Output.TexCoord = float2(Tile.TexCoord[IndexX], Tile.TexCoord[IndexY]);
    Output.Color = Tile.Color;
    Output.PolyFlags = Tile.PolyFlags;
    return Output;
}

float3 PSMain(const VSOut Input) : SV_Target
{  

    const float4 Diffuse = TexDiffuse[Input.PolyFlags.y].Sample(SamLinear, Input.TexCoord);
    if (Input.PolyFlags.x & PF_Masked || Input.PolyFlags.x & PF_Modulated) // Not sure if Modulated should be clipped, but so far UI elements that I've seen only use 1 bit alpha
        clip(Diffuse.a - 0.5f);
    const float3 Color = Diffuse.rgb * Input.Color;

    return Color;
}
