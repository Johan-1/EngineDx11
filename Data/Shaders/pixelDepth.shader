Texture2D Texture;
SamplerState SampleType[6]; // wrapTrilinear, clampTrilinear, wrapBilinear, clampBililinear, wrapAnisotropic, clampAnisotropic

struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

void Main(PixelInputType input)
{  	
	if (Texture.Sample(SampleType[0], input.tex).a < 0.7)
		discard;	   	    
}