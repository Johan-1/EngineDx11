uniform matrix u_viewProj;

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex      : TEXCOORD0;
	float3 normal   : NORMAL;
	float3 tangent  : TANGENT;
	float3 binormal : BINORMAL;
	float4 color    : COLOR;
	
	// per instance
	float4 worldM1 : TEXCOORD1;
	float4 worldM2 : TEXCOORD2;
	float4 worldM3 : TEXCOORD3;
	float4 worldM4 : TEXCOORD4;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 tex      : TEXCOORD0;
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;   

    // calculate world matrix
	float4x4 world = float4x4(input.worldM1, input.worldM2, input.worldM3, input.worldM4);
	float4x4 worldViewProj = mul(world, u_viewProj);
	
	// calculate output position
    input.position.w = 1.0f;
	output.position = mul(input.position, worldViewProj);
    output.tex      = input.tex;
	   
    return output;
}