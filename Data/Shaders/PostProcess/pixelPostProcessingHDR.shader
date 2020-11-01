Texture2D    Texture[4];    // ScenemapHDR, bloomMap, dofmap, sceneDepth 
SamplerState SampleType[6]; // wrapTrilinear, clampTrilinear, wrapBilinear, clampBililinear, wrapAnisotropic, clampAnisotropic	

uniform int    u_applyBloom;
uniform float  u_bloomIntensity;
uniform int    u_applyDof;
uniform int    u_applyToneMapping;
uniform float2 u_dofStartEndDst;
uniform int    u_tonemapType;
uniform float  u_tonemap3Exposure;

static float nearPlane = 0.1;
static float farPlane  = 5000.0;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;		
};

float4 CalculateBloomColor(float2 texCoord)
{
	return Texture[1].Sample(SampleType[1], texCoord);
}

float InverseLerp(float a, float b, float t)
{
	return saturate((t - a) / (b - a));
}

float GetLinearDeapth(float pixeldepth)
{
	return 2.0 * nearPlane * farPlane / (farPlane + nearPlane - (2.0 * pixeldepth - 1.0) * (farPlane - nearPlane));
}

// tonemapping by naughty dog used in uncharted 2
float3 Tonemap1(float3 hdrColor)
{
	float3 x = 2.0 * hdrColor; 
	float3 newColor = ((x * (0.15 * x + 0.05) + 0.004) / (x * (0.15 * x + 0.5) + 0.06)) - 0.02/0.3;
	return newColor * 1.3790642467;
}

// Reinhard tonemapping 
float3 Tonemap2(float3 hdrColor)
{
	return hdrColor / (1.0 + hdrColor);
}

// Simple exposure-based tonemapping
float3 Tonemap3(float3 hdrColor, float exposure)
{
	return 1.0 - exp(-hdrColor * exposure);
}

float4 Main(PixelInputType input) : SV_TARGET
{          
	float4 bloomColor = float4(0,0,0,1);	
	float4 baseColor  = Texture[0].Load(int3(input.position.xy,0));
	float4 dofColor   = Texture[2].Load(int3(input.position.xy,0));
	float4 depth      = Texture[3].Load(int3(input.position.xy,0));
	float4 finalColor = float4(0,0,0,1);
	float dofFraction = 0.0f;
	
	if (u_applyBloom == 1) bloomColor  = CalculateBloomColor(input.texCoord);
	if (u_applyDof == 1)   dofFraction = InverseLerp(u_dofStartEndDst.x, u_dofStartEndDst.y, GetLinearDeapth(depth.x));
		
	baseColor  = lerp(baseColor, dofColor, dofFraction);	
	finalColor = baseColor + (bloomColor * u_bloomIntensity);	
	
	if (u_applyToneMapping)
	{
		if      (u_tonemapType == 0) finalColor.rgb = Tonemap1(finalColor.rgb);
		else if (u_tonemapType == 1) finalColor.rgb = Tonemap2(finalColor.rgb);
		else if (u_tonemapType == 2) finalColor.rgb = Tonemap3(finalColor.rgb, u_tonemap3Exposure);
	}

	return finalColor;
}






























