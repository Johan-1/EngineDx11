#include "CommonPixel.shader"
Texture2D    Texture[3];      // diffuse, normal, emissive
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

cbuffer heightBuffer : register(b2)
{
	float4 cameraPosition;
	int    hasHeightmap; 	
	float  heightScale;
}
 
struct PixelInputType
{
    float4 position    : SV_POSITION;
    float2 texCoord    : TEXCOORD0;
	float3 normal      : NORMAL;
	float3 tangent     : TANGENT;
	float3 binormal    : BINORMAL;
	float4 vertexColor : COLOR;
	float3 worldPos    : TEXCOORD1;
	float clip         : SV_ClipDistance0;	
};

float3 GetBaseColor(PixelInputType input, float3 textureColor)
{
	return (textureColor * input.vertexColor.rgb) * ambientColor.rgb;
}

float3 GetDirectionalColor(PixelInputType input,float3 textureColor, float3 bumpNormal)
{										  						
    float lightIntensity = saturate(dot(bumpNormal, dirLightDirection)); 
    return textureColor * input.vertexColor.rgb * lightIntensity * dirDiffuseColor.rgb; 
}

float3 GetPointColor(PixelInputType input, float3 textureColor, float3 bumpNormal)
{   
	float3 finalColor = float3(0,0,0);
	int numLights     = pointLights[0].numLights;
	
	for (int i =0; i < numLights; i++)
	{	
		// get light direction and distance from light	
		float3 lightDir   = -normalize(input.worldPos - pointLights[i].position);	
		float dst         = length(input.worldPos - pointLights[i].position);
		float fallOff     = 1 - ((dst / pointLights[i].radius) * 0.5);	
		float attuniation = 1 / (pointLights[i].attConstant + pointLights[i].attLinear * dst + pointLights[i].attExponential * dst * dst);
		attuniation       = (attuniation * pointLights[i].intensity) * fallOff;
		
		if (fallOff >= 0)
		{								
			float lightIntensity = saturate(dot(bumpNormal, lightDir)); 						
			float3 color = textureColor * input.vertexColor.rgb * lightIntensity * pointLights[i].color; 							
			finalColor += (color * attuniation);
		}
	}
     
    return finalColor;
}

float4 Main(PixelInputType input) : SV_TARGET
{     	         
	
	input.normal   = normalize(input.normal);
	input.binormal = normalize(input.binormal);
	input.tangent  = normalize(input.tangent);
	
	float3x3 TBNMatrix = float3x3(input.tangent,input.binormal,input.normal);
	
	if (hasHeightmap == 1)
		input.texCoord += GetPOMOffset(Texture[1], SampleType[0], input.normal, input.tangent, input.worldPos, cameraPosition.xyz, input.texCoord, heightScale);

	float4 textureColor  = Texture[0].Sample(SampleType[0], input.texCoord);		
	float4 normalMap     = Texture[1].Sample(SampleType[0], input.texCoord);
	float4 emissiveColor = Texture[2].Sample(SampleType[0], input.texCoord);
	
	if (emissiveColor.r + emissiveColor.g + emissiveColor.b > 0.1)
		return float4(emissiveColor.rgb * 1.1, 1);
	
	// convert normalmap sample to range -1 to 1 and convert to worldspace
	normalMap         = (normalMap * 2.0) -1.0;		
	float3 bumpNormal = normalize(mul(normalMap, TBNMatrix));  
		
	float3 baseColor        = GetBaseColor(input, textureColor);		
	float3 directionalColor = GetDirectionalColor(input, textureColor, bumpNormal);
	float3 pointColor       = GetPointColor(input, textureColor, bumpNormal);

	return float4(baseColor + directionalColor + pointColor, textureColor.a);
}