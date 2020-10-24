uniform float4x4 u_worldMatrix;
uniform float4x4 u_worldViewProj;
uniform float4 u_clippingPlane;
uniform float2 u_uvOffset;

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
    float4 position    : SV_POSITION;
    float2 texCoord    : TEXCOORD0;
	float3 normal      : NORMAL;
	float3 tangent     : TANGENT;
	float3 binormal    : BINORMAL;
	float4 vertexColor : COLOR;
	float3 worldPos    : TEXCOORD1;
	float  clip        : SV_ClipDistance0;	
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;   

	// send vertex colors and uvs
	output.vertexColor = input.color;
	output.texCoord    = input.texCoord + u_uvOffset;
	
    input.position.w = 1.0f;
    output.position  = mul(input.position, u_worldViewProj);
	
	// transform normals to worldSpace
	output.normal   = normalize(mul(input.normal,   (float3x3)u_worldMatrix));
	output.tangent  = normalize(mul(input.tangent,  (float3x3)u_worldMatrix));
	output.binormal = normalize(mul(input.binormal, (float3x3)u_worldMatrix));
	
	// get the direction from vertex to camera for specular calculations	
	float4 worldPosition  = mul(input.position, u_worldMatrix);	
	output.worldPos  = worldPosition;
	
	// create the clipping plane to clip pixels under the reflection surface
	output.clip = dot(mul(input.position, u_worldMatrix), u_clippingPlane);
	
	  
    return output;
}