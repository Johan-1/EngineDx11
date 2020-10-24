uniform matrix u_worldMatrix;
uniform matrix u_viewMatrix;
uniform matrix u_projectionMatrix;

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
	float2 texCoord : TEXCOORD0;
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;   

    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;	
   
	// get the worldViewmatrix
	float4x4 worldViewMatrix = mul(u_worldMatrix, u_viewMatrix);

	// get the vertices positions in screen space
	// this do not consider the scale or rotation 
	// from the world matrix but we dont need those
    float4 positionViewSpace = float4(worldViewMatrix._41, worldViewMatrix._42, worldViewMatrix._43, 0) + input.position;
	
    output.position = mul(positionViewSpace, u_projectionMatrix);
	output.texCoord = input.texCoord;
	
    return output;
}