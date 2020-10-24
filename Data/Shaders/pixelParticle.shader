Texture2D diffuseMap;	
SamplerState SampleType[6]; // wrapTrilinear, clampTrilinear, wrapBilinear, clampBililinear, wrapAnisotropic, clampAnisotropic
 
struct PixelInputType
{
	float4 position : SV_POSITION;
    float2 tex      : TEXCOORD0;		
	float4 color    : COLOR;
};

float4 Main(PixelInputType input) : SV_TARGET
{            
   return diffuseMap.Sample(SampleType[0], input.tex) * input.color; 	   
}