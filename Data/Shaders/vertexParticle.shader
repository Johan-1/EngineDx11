uniform matrix u_viewProj;

struct VertexInputType
{
	// per vertexdata
    float4 position : POSITION;
    float2 tex      : TEXCOORD0;
	
	// per instance data
	float4 worldM1 : TEXCOORD1;
	float4 worldM2 : TEXCOORD2;
	float4 worldM3 : TEXCOORD3;
	float4 worldM4 : TEXCOORD4;
	float4 color   : COLOR;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex      : TEXCOORD0;		
	float4 color    : COLOR;
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;   

    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;
	
	float4x4 w = float4x4(input.worldM1, input.worldM2, input.worldM3, input.worldM4);
   
    output.position = mul(input.position, w); 	
    output.position = mul(output.position, u_viewProj); 
    
	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;
	
	output.color = input.color;
	  
    return output;
}