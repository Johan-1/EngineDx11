uniform float4x4 u_viewProj;
uniform float4 u_clippingPlane;

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
	output.texCoord    = input.texCoord;
	
    // calculate world and worldViewproj matrix
	float4x4 world         = float4x4(input.worldM1, input.worldM2, input.worldM3, input.worldM4);
	float4x4 worldViewProj = mul(world, u_viewProj);
	
	// calculate output position
    input.position.w = 1.0f;
	output.position  = mul(input.position, worldViewProj);
	
	// transform normals to worldSpace
	output.normal   = normalize(mul(input.normal,   (float3x3)world));
	output.tangent  = normalize(mul(input.tangent,  (float3x3)world));
	output.binormal = normalize(mul(input.binormal, (float3x3)world));
	
	// get the direction from vertex to camera for specular calculations	
	float4 worldPosition  = mul(input.position, world);	
	output.worldPos = worldPosition;
	
	// create the clipping plane to clip pixels under the reflection surface
	output.clip = dot(mul(input.position, world), u_clippingPlane);
	  
    return output;
}