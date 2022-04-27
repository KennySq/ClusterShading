cbuffer DefaultConstants : register(b0)
{
    float4x4 World;
    float4x4 VP;
}

struct Vertex
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float3 Binormal : BINORMAL0;
    float3 Tangent : TANGENT0;
    float2 Texcoord : TEXCOORD0;
};

struct Pixel
{
    float4 Position : SV_Position;
    float4 Normal : TEXCOORD0;
    float4 Binormal : TEXCOORD1;
    float4 Tangent : TEXCOORD2;
    float4 WorldNormal : TEXCOORD3;
    float2 Texcoord : TEXCOORD4;
};

Pixel vert(Vertex input)
{
    Pixel output = (Pixel) 0;
    
    output.Position = mul(float4(input.Position, 1.0f), World);
    output.Position = mul(output.Position, VP);
    
    output.Normal = float4(input.Normal, 1.0f);
    output.WorldNormal = float4(mul(input.Normal, (float3x3) World), 1.0f);
    output.Binormal = float4(input.Binormal, 1.0f);
    output.Tangent = float4(input.Tangent, 1.0f);
    output.Texcoord = input.Texcoord;

	return output;
}

float4 frag(Pixel input) : SV_Target0
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);

}