#include "CommonPixel.shader"

Texture2D Texture[7];	    // color, normal, metalic, roughness, emissive, shadowMap, reflectionMap
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
	float4 dirDiffuseColor;
	float3 dirLightDirection;
}

cbuffer PointLightBuffer : register(b1)
{
	PointLight pointLights[1024];
};

cbuffer ReflectionBuffer : register(b2)
{
	float reflectiveFraction;
	float3 cameraPosition;
	float metalicAmount;
	float roughnessAmount;
}

struct PixelInputType
{
    float4 position           : SV_POSITION;
    float2 texCoord           : TEXCOORD0;
	float3 normal             : NORMAL;
	float3 tangent            : TANGENT;
	float3 binormal           : BINORMAL;
	float4 positionLightSpace : TEXCOORD1;
	float3 worldPos           : TEXCOORD2;
	float4 vertexColor        : COLOR;
	float4 reflectionPosition : TEXCOORD3;
};

float3 GetBaseColor(PixelInputType input, float3 diffuseColor)
{
	return (diffuseColor * input.vertexColor.rgb) * ambientColor.rgb;
}

float3 GetDirectionalColor(PixelInputType input, float4 baseColor, float4 normal, float4 pbr)
{
	// get how much the pixel is in light 
	float lightPercent = GetShadowLightFraction(Texture[5], SampleType[1], input.positionLightSpace, 0.00001);
	if (lightPercent == 0)
	{
		return float3(0, 0, 0);
	}

	return GetLightRadiance(float4(input.worldPos, 0), baseColor, normal, pbr, cameraPosition.xyz, dirLightDirection, dirDiffuseColor.rgb);
}

float3 GetPointColor(PixelInputType input, float4 baseColor, float4 normal, float4 pbr)
{
	float3 finalColor = float3(0, 0, 0);
	int numLights = pointLights[0].numLights;

	for (int i = 0; i < numLights; i++)
	{
		// get light direction and distance from light	
		float3 lightDir   = -normalize(input.worldPos - pointLights[i].position);
		float dst         = length(input.worldPos - pointLights[i].position);
		float fallOff     = 1 - ((dst / pointLights[i].radius) * 0.5);
		float attuniation = 1 / (pointLights[i].attConstant + pointLights[i].attLinear * dst + pointLights[i].attExponential * dst * dst);
		attuniation       = (attuniation * pointLights[i].intensity) * fallOff;

		if (fallOff >= 0)
		{
			float lightIntensity = saturate(dot(normal.xyz, lightDir));
			float3 color = baseColor.rgb * input.vertexColor.rgb * lightIntensity * pointLights[i].color * attuniation;

			finalColor += GetLightRadiance(float4(input.worldPos, 0), baseColor, normal, pbr, cameraPosition, lightDir, color.rgb);
		}
	}

	return finalColor;
}

float4 GetReflectionColor(PixelInputType input)
{
	float2 reflectionTexCoords;
	
	// get the projected texture coordinates so we can sample the reflection map
	reflectionTexCoords.x =  input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
    reflectionTexCoords.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;
	
	return Texture[6].Sample(SampleType[0], reflectionTexCoords);
}

float4 Main(PixelInputType input) : SV_TARGET
{
	input.normal   = normalize(input.normal);
	input.binormal = normalize(input.binormal);
	input.tangent  = normalize(input.tangent);

	float3x3 TBNMatrix = float3x3(input.tangent,input.binormal,input.normal);

	float4 textureColor = Texture[0].Sample(SampleType[0], input.texCoord);
	float4 normalMap    = Texture[1].Sample(SampleType[0], input.texCoord);
	float4 metalicMap   = Texture[2].Sample(SampleType[0], input.texCoord) * metalicAmount;
	float4 roughnessMap = Texture[3].Sample(SampleType[0], input.texCoord) * roughnessAmount;
	
	// convert normalmap sample to range -1 to 1 and convert to worldspace
	normalMap = (normalMap * 2.0) - 1.0;
	float3 bumpNormal = normalize(mul(normalMap, TBNMatrix));

	float4 reflectionColor  = GetReflectionColor(input);
	float3 baseColor        = GetBaseColor(input, textureColor.rgb);
	float3 directionalColor = GetDirectionalColor(input, textureColor, float4(bumpNormal, 0), float4(metalicMap.r, roughnessMap.r, 0, 0));
	float3 pointColor       = GetPointColor(input, textureColor, float4(bumpNormal, 0), float4(metalicMap.r, roughnessMap.r, 0, 0));
	
	float3 finalColor = baseColor + directionalColor + pointColor;

	finalColor.rgb = finalColor.rgb / (1.0 + finalColor.rgb);

	finalColor = lerp(finalColor, reflectionColor, reflectiveFraction);
	
	return float4(finalColor.rgb, textureColor.a);
}
