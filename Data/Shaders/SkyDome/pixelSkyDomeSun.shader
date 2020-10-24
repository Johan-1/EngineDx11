Texture2D    Texture;
SamplerState SampleType[6]; // wrapTrilinear, clampTrilinear, wrapBilinear, clampBililinear, wrapAnisotropic, clampAnisotropic	

uniform float  u_sunDot;
uniform float3 u_sunTint;
uniform float2 u_beginEndFade;
 
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;		
};

float GetSunStrenghtFraction(float a, float b, float t)
{
	return saturate((t - a) / (b - a));
}

float4 Main(PixelInputType input) : SV_TARGET
{     	           	 		
	 float4 color = Texture.Sample(SampleType[0], input.texCoord);
	 	 
	 color = (color * float4(u_sunTint.rgb, 1.0)) * GetSunStrenghtFraction(u_beginEndFade.y, u_beginEndFade.x, u_sunDot);
	 
	 return color;
}