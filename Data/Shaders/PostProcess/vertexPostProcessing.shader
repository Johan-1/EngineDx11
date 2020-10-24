
struct VertexInputType
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;   
       	   	
    output.position = input.position;
    output.texCoord = input.texCoord;
	   
    return output;
}