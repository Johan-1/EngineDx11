#include "CommonPixel.shader"

Texture2D    Texture[5];    // shadowmap, position, normal, baseColor, pbr	
SamplerState SampleType[6]; // wrapTrilinear, clampTrilinear, wrapBilinear, clampBililinear, wrapAnisotropic, clampAnisotropic 	
 
struct PointLight
{
	float3 position;
	float radius;
	float3 color;
	float intensity;	
	float attConstant;
	float attLinear;
	float attExponential;
	int numLights;
};

cbuffer AmbientDirectionalBuffer : register(b0)
{
	float4 ambientColor;
	float4 lightColorDir;
	float3 lightDirectionDir;
}

cbuffer PointLightBuffer : register(b1)
{
	PointLight pointLights[1024];
};

cbuffer MiscBuffer : register(b2)
{
	float4x4 lightViewProj;
	float4   cameraPosition;
}

struct PixelInputType
{
    float4 position : SV_POSITION;	
};

float3 GetAmbientColor(float4 diffuseMap)
{
	return (diffuseMap * ambientColor).rgb;
}

float3 GetDirectionalColor(float4 position, float4 baseColor, float4 normal, float4 pbr)
{	
	float4 positionLightSpace = mul(position, lightViewProj);
	
	// get how much the pixel is in light 
	float lightPercent = GetShadowLightFraction(Texture[0], SampleType[1], positionLightSpace, 0.00001);
	if (lightPercent == 0)
	{
		return float3(0,0,0);
	}

	return GetLightRadiance(position, baseColor * lightPercent, normal, pbr, cameraPosition.xyz, lightDirectionDir, lightColorDir.rgb);
}

float3 GetPointColor(float4 position, float4 baseColor, float4 normal, float4 pbr)
{
	float3 finalColor = float3(0,0,0);
	int numLights     = pointLights[0].numLights;
	
	for (int i = 0; i < numLights; i++)
	{				
		float3 lightDir   = -normalize(position.xyz - pointLights[i].position);
		float dst         = length(position.xyz - pointLights[i].position);
		float fallOff     = 1 - ((dst / pointLights[i].radius) * 0.5);
		float attuniation = 1 / (pointLights[i].attConstant + pointLights[i].attLinear * dst + pointLights[i].attExponential * dst * dst);
		attuniation       = (attuniation * pointLights[i].intensity) * fallOff;
		
		if (fallOff >= 0)
		{
			float lightIntensity = saturate(dot(normal.xyz, lightDir)); 
			float4 color = baseColor * lightIntensity * float4(pointLights[i].color, 1) * attuniation;

			finalColor += GetLightRadiance(position, baseColor, normal, pbr, cameraPosition.xyz, lightDir, color.rgb);
		}
	}
      
    return finalColor;
}

float4 Main(PixelInputType input) : SV_TARGET
{            
	// alpha channel is flagged as 0 in the diffusemap if this is an emissive pixel
	// dont do any lightning calculations and only return the emissive
	// color that is stored in the baseColor map
	float4 baseColor = Texture[3].Load(int3(input.position.xy,0));
	if (baseColor.a == 0)	
		return baseColor;
		
	float4 position = Texture[1].Load(int3(input.position.xy,0));
	float4 normal   = Texture[2].Load(int3(input.position.xy,0));
	float4 pbr      = Texture[4].Load(int3(input.position.xy,0));
		
	float4 finalColor = float4(0,0,0,1);

	float3 ambientColor     = GetAmbientColor(baseColor);
	float3 directionalColor = GetDirectionalColor(position, baseColor, normal, pbr);
	float3 pointColor       = GetPointColor(position, baseColor, normal, pbr);
	
	finalColor.rgb = ambientColor + directionalColor + pointColor;
	
	// do rainhart tonemapping
	finalColor.rgb = finalColor.rgb / (1.0 + finalColor.rgb);
	
    return finalColor;
}






























