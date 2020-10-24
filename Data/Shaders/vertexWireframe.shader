uniform matrix u_worldViewProj;

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex      : TEXCOORD0;
	float3 normal   : NORMAL;
	float3 tangent  : TANGENT;
	float3 binormal : BINORMAL;
	float4 color    : COLOR;	
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;   

    input.position.w = 1.0f;   
    output.position  = mul(input.position, u_worldViewProj); 
    output.color     = input.color;
	  
    return output;
}