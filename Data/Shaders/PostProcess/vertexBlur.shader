uniform float u_screenWidth;
uniform float u_screenHeight;
uniform int u_horizontalPass;

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex      : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex1     : TEXCOORD0;
    float2 tex2     : TEXCOORD1;
    float2 tex3     : TEXCOORD2;
    float2 tex4     : TEXCOORD3;
    float2 tex5     : TEXCOORD4;
    float2 tex6     : TEXCOORD5;
    float2 tex7     : TEXCOORD6;
    float2 tex8     : TEXCOORD7;
    float2 tex9     : TEXCOORD8;
    float2 tex10    : TEXCOORD9;
    float2 tex11    : TEXCOORD10;	
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;   
       	   	
	output.position = input.position;
    float2 offset = u_horizontalPass ? float2(1 / u_screenWidth, 0) : float2(0, 1 / u_screenHeight);	

	output.tex1  = input.tex + (offset * - 5);
	output.tex2  = input.tex + (offset * - 4);
	output.tex3  = input.tex + (offset * - 3);
	output.tex4  = input.tex + (offset * - 2);
	output.tex5  = input.tex + (offset * - 1);
	output.tex6  = input.tex + (offset *   0);
	output.tex7  = input.tex + (offset *   1);
	output.tex8  = input.tex + (offset *   2);
	output.tex9  = input.tex + (offset *   3);
	output.tex10 = input.tex + (offset *   4);
	output.tex11 = input.tex + (offset *   5);
		   
    return output;
}