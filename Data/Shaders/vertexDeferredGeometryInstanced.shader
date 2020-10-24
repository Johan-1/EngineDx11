uniform float4x4 u_viewProj;

struct VertexInputType
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
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
    float4 position      : SV_POSITION;
    float2 texCoord      : TEXCOORD0;
	float3 normal        : NORMAL;
	float3 tangent       : TANGENT;
	float3 binormal      : BINORMAL;
	float4 worldPosition : TEXCOORD1;
	float4 color         : COLOR;
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;
	
	// send texcoords to pixelshader
	output.texCoord = input.texCoord;
	  
	// calculate world matrix
	float4x4 world = float4x4(input.worldM1, input.worldM2, input.worldM3, input.worldM4);
	float4x4 worldViewProj = mul(world, u_viewProj);
	
	// calculate output position
    input.position.w = 1.0f;
	output.position = mul(input.position, worldViewProj); 	
	 	
	// transform normals to worldSpace
	output.normal   = normalize(mul(input.normal,   (float3x3)world));
	output.tangent  = normalize(mul(input.tangent,  (float3x3)world));
	output.binormal = normalize(mul(input.binormal, (float3x3)world));
	
	output.worldPosition = mul(input.position, world);
	
	output.color = input.color;
		   
    return output;
}