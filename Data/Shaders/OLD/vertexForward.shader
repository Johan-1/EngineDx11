uniform float4x4 u_worldMatrix;
uniform float4x4 u_worldViewProj;
uniform float4x4 u_worldViewProjLight;
uniform float3 	 u_cameraPosition;
uniform float pad;
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
    float4 position           : SV_POSITION;
    float2 texCoord           : TEXCOORD0;
	float3 normal             : NORMAL;
	float3 tangent            : TANGENT;
	float3 binormal           : BINORMAL;
	float3 vertexToCamera     : TEXCOORD1;
	float4 positionLightSpace : TEXCOORD2;	 
	float3 worldPos           : TEXCOORD3;
	float4 vertexColor        : COLOR;
	float3 cameraPos          : TEXCOORD4;
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;
	
	output.texCoord  = input.texCoord + u_uvOffset;	
	input.position.w = 1.0f;
    output.position  = mul(input.position, u_worldViewProj); 	
    	   	
	// transform normals to worldSpace
	output.normal   = normalize(mul(input.normal,   (float3x3)u_worldMatrix));	     	
	output.tangent  = normalize(mul(input.tangent,  (float3x3)u_worldMatrix));		
	output.binormal = normalize(mul(input.binormal, (float3x3)u_worldMatrix));
	
	// transform position into the space of the directional light that will calculate the shadows
	output.positionLightSpace = mul(input.position, u_worldViewProjLight); 	

	// get the direction from vertex to camera for specular calculations	
	float4 worldPosition  = mul(input.position, u_worldMatrix);
	output.vertexToCamera = u_cameraPosition.xyz - worldPosition.xyz;
	
	// send world pos to pixel shader for point light calculations
	output.worldPos = worldPosition;
	
	output.vertexColor = input.color;	
	output.cameraPos   = u_cameraPosition;
		   
    return output;
}













