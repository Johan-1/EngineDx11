Texture2D    Texture; 
SamplerState SampleType[6]; // wrapTrilinear, clampTrilinear, wrapBilinear, clampBililinear, wrapAnisotropic, clampAnisotropic	

static float weights[11] =
{
	0.0093,
	0.028002,
	0.065984,
	0.121703,
	0.175713,
	0.198596,
	0.175713,
	0.121703,
	0.065984,
	0.028002,
	0.0093
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

float4 Main(PixelInputType input) : SV_TARGET
{
	float4 color = float4(0,0,0,1);
	
	color += Texture.Sample(SampleType[1], input.tex1)  * weights[0];
	color += Texture.Sample(SampleType[1], input.tex2)  * weights[1];
	color += Texture.Sample(SampleType[1], input.tex3)  * weights[2];
	color += Texture.Sample(SampleType[1], input.tex4)  * weights[3];
	color += Texture.Sample(SampleType[1], input.tex5)  * weights[4];
	color += Texture.Sample(SampleType[1], input.tex6)  * weights[5];
	color += Texture.Sample(SampleType[1], input.tex7)  * weights[6];
	color += Texture.Sample(SampleType[1], input.tex8)  * weights[7];
	color += Texture.Sample(SampleType[1], input.tex9)  * weights[8];
	color += Texture.Sample(SampleType[1], input.tex10) * weights[9];
	color += Texture.Sample(SampleType[1], input.tex11) * weights[10];
	
	return color;	
}






























