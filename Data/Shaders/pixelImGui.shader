Texture2D shaderTexture;	
SamplerState SampleType[6]; // wrapTrilinear, clampTrilinear, wrapBilinear, clampBililinear, wrapAnisotropic, clampAnisotropic	
 
struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
	float2 uv  : TEXCOORD0;
};
   
float4 Main(PS_INPUT input) : SV_TARGET
{
	float4 out_col = input.col * shaderTexture.Sample(SampleType[0], input.uv); 
	return out_col; 
};