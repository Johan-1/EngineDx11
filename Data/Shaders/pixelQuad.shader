Texture2D shaderTexture;	
SamplerState SampleType[6]; // wrapTrilinear, clampTrilinear, wrapBilinear, clampBililinear, wrapAnisotropic, clampAnisotropic	
 
uniform float4 u_color;
uniform int    u_ignoreAlpha;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex      : TEXCOORD0;		
};

float4 Main(PixelInputType input) : SV_TARGET
{ 
    float4 color = shaderTexture.Sample(SampleType[0], input.tex) * u_color;
	
	if (u_ignoreAlpha) color.a = 1.0f;
		
	return color;		    
}