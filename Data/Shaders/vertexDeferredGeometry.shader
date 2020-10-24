uniform float4x4 u_worldMatrix;
uniform float4x4 u_worldViewProj;
uniform float2   u_uvOffset;

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
	output.texCoord = input.texCoord + u_uvOffset;
	  
    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;
	
	// get position acording to worldViewProjection
    output.position = mul(input.position, u_worldViewProj); 	
    	   	
	// transform normals to worldSpace
	output.normal   = normalize(mul(input.normal,   (float3x3)u_worldMatrix));
	output.tangent  = normalize(mul(input.tangent,  (float3x3)u_worldMatrix));
	output.binormal = normalize(mul(input.binormal, (float3x3)u_worldMatrix));
	
	output.worldPosition = mul(input.position, u_worldMatrix);
	
	output.color = input.color;

    return output;
}