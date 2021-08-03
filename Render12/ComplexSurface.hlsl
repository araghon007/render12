#include "Render12.hlsli"

Texture2D TexDiffuse[1024] : register(t0);
//Texture2D TexLight : register(t1);

struct SPoly
{
    float4 Pos : Position0;
    float2 TexCoord : TexCoord0;
    float2 TexCoord1 : TexCoord1;
    uint3 PolyFlags : BlendIndices0;
};

struct VSOut
{
    float4 Pos : SV_Position;
    float2 TexCoord : TexCoord0;
    float2 TexCoord1 : TexCoord1;
    uint3 PolyFlags : BlendIndices0;
};

VSOut VSMain(const SPoly Input)
{
    VSOut Output;
    Output.Pos = mul(Input.Pos, ProjectionMatrix);
    Output.TexCoord = Input.TexCoord;
    Output.TexCoord1 = Input.TexCoord1;
    Output.PolyFlags = Input.PolyFlags;
    return Output;
}

float4 PSMain(const VSOut Input) : SV_Target
{
    if (Input.PolyFlags.x & PF_Masked)
    {
      //  clip(TexDiffuse.Sample(SamPoint, Input.TexCoord).a - 0.5f);
    }

    float4 Color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    if (Input.PolyFlags.y & 0x00000001)
    {
        const float3 Diffuse = TexDiffuse[Input.PolyFlags.z].Sample(SamLinear, Input.TexCoord).rgb;
        Color.rgb *= Diffuse;
    }
    if (Input.PolyFlags.y & 0x00000002)
    {
        clip(0);
        //const float3 Light = TexLight.Sample(SamLinear, Input.TexCoord1).bgr * 2.0f;
        //Color.rgb *= Light;
    }
    

    return Color;
}
