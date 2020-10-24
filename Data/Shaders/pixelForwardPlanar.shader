#include "CommonPixel.shader"

Texture2D Texture[6];	    // diffuse, normal, specular, emissive, shadowMap, reflectionMap
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
}

struct PixelInputType
{
    float4 position           : SV_POSITION;
    float2 texCoord           : TEXCOORD0;
	float3 normal             : NORMAL;
	float3 tangent            : TANGENT;
	float3 binormal           : BINORMAL;
	float3 vertexToCamera     : TEXCOORD1;
	float4 positionLightSpace : TEXCOORD2;
	float3 worldPos           : TEXCOORD3;
	float4 vertexColor        : COLOR;
	float4 reflectionPosition : TEXCOORD4;
};

float3 GetBaseColor(PixelInputType input, float3 diffuseColor)
{
	return (diffuseColor * input.vertexColor.rgb) * ambientColor.rgb;
}

float3 GetDirectionalColor(PixelInputType input, float4 diffuse, float3 normal, float4 specularMap)
{	
	// get how much the pixel is in light 
	float lightPercent = GetShadowLightFraction(Texture[4], SampleType[1], input.positionLightSpace, 0.00001);
	if (lightPercent == 0)
		return float3(0,0,0);
										  
	// sample from textures, multiply texturecolor with light percent to get blured shadow edges		
	float4 textureColor = diffuse * lightPercent;
			 
	// get light intensity			 
    float lightIntensity = saturate(dot(normal, dirLightDirection)); 
	
	// get color based on texture, intensity and color of light
    float4 color = (textureColor * lightIntensity * dirDiffuseColor); 
	
	float4 specular = float4(0,0,0,0);		
	if (lightIntensity > 0)
		specular = GetSpecularColorPhong(normalize(input.vertexToCamera), dirLightDirection, dirDiffuseColor, lightIntensity, normal, specularMap);
			
	color += specular;		
	return color.rgb; 
}

float3 GetPointColor(PixelInputType input, float3 textureColor, float3 bumpNormal, float4 specularMap)
{   
	float3 finalColor = float3(0,0,0);
	int numLights     = pointLights[0].numLights;
	
	for (int i =0; i < numLights; i++)
	{	
		// get light direction and distance from light	
		float3 lightDir = -normalize(input.worldPos - pointLights[i].position);	
		float dst       = length(input.worldPos - pointLights[i].position);
		
		// calculate attuniation	
		float fallOff     = 1 - ((dst / pointLights[i].radius) * 0.5);	// make the falloff 50% slower then the radius to give more control of result depending of attuniation settings
		float attuniation = 1 / (pointLights[i].attConstant + pointLights[i].attLinear * dst + pointLights[i].attExponential * dst * dst);
		attuniation       = (attuniation * pointLights[i].intensity) * fallOff;
		
		if (fallOff >= 0)
		{						
			// get intensity from the normal from normalmap and lightdirection
			float lightIntensity = saturate(dot(bumpNormal, lightDir)); 
			
			// get color based on texture, intensity and color of light
			float3 color = textureColor * input.vertexColor.rgb * lightIntensity * pointLights[i].color; 
						
			float3 specular = float3(0,0,0);		
			if (lightIntensity > 0 && dst < pointLights[i].radius)
			{		
				float3 vtc              = normalize(input.vertexToCamera);
				float3 reflection       = normalize(2 * lightIntensity * bumpNormal - lightDir);		
				float specularIntensity = pow(saturate(dot(reflection, vtc)), specularMap.a * 255.0);
				specular                = (pointLights[i].color * specularIntensity) * specularMap.rgb;	
			} 
						
			color      += specular;	
			finalColor += (color * attuniation);
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
	
	return Texture[5].Sample(SampleType[0], reflectionTexCoords);
}

float4 Main(PixelInputType input) : SV_TARGET
{	
	float4 textureColor = Texture[0].Sample(SampleType[0], input.texCoord);		
	float4 normalMap    = Texture[1].Sample(SampleType[0], input.texCoord);
	float4 specularMap  = Texture[2].Sample(SampleType[0], input.texCoord);
	
	// convert normalmap sample to range -1 to 1 and convert to worldspace
	normalMap                   = (normalMap * 2.0) -1.0;
	float3x3 tangentSpaceMatrix = float3x3(input.tangent,input.binormal,input.normal);	
	float3 bumpNormal           = normalize(mul(normalMap, tangentSpaceMatrix));  

	float4 reflectionColor  = GetReflectionColor(input);
	float3 baseColor        = GetBaseColor(input, textureColor.rgb);
	float3 directionalColor = GetDirectionalColor(input, textureColor, bumpNormal, specularMap);
	float3 pointColor       = GetPointColor(input, textureColor.rgb, bumpNormal, specularMap);
	
	float reflectFraction = reflectiveFraction;
	
	float3 finalColor = baseColor + directionalColor + pointColor; 
	finalColor = lerp(finalColor, reflectionColor, reflectFraction);
	
	return float4(finalColor.rgb, textureColor.a); 
}






