TextureCube  Texture;
SamplerState SampleType[6]; // wrapTrilinear, clampTrilinear, wrapBilinear, clampBililinear, wrapAnisotropic, clampAnisotropic   	
 
struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;		
};

float4 Main(PixelInputType input) : SV_TARGET
{     	           	 		
	 return input.color;
}