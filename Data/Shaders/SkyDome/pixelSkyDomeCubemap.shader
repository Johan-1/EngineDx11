TextureCube  Texture;
SamplerState SampleType[6]; // wrapTrilinear, clampTrilinear, wrapBilinear, clampBililinear, wrapAnisotropic, clampAnisotropic 	
 
struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 tex      : TEXCOORD0;		
};

float4 Main(PixelInputType input) : SV_TARGET
{     	           	 		
	 return Texture.Sample(SampleType[1], input.tex);
}