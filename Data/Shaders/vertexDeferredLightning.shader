
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex      : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;   
       	   	
	output.position = input.position;
   	   
    return output;
}