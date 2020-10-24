uniform matrix u_worldViewProj;

struct VertexInputType
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
	float3 normal   : NORMAL;
	float3 tangent  : TANGENT;
	float3 binormal : BINORMAL;
	float4 color    : COLOR;	
};

struct PixelInputType
{
    float4 position : SV_POSITION;	
	float  height   : TEXCOORD0;
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;   

    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;
   
   // mutiply all matrices
    output.position = mul(input.position, u_worldViewProj);
	
	output.height = input.position.y;
	  
    return output;
}