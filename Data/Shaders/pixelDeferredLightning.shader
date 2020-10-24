#include "CommonPixel.shader"

Texture2D    Texture[5];    // shadowmap, position, normal, diffuse, specular	
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

float4 GetAmbientColor(float4 diffuseMap)
{
	return diffuseMap * ambientColor;
}

float4 GetDirectionalColor(float4 position, float4 diffuse, float4 normal, float4 specularMap)
{	
	float4 positionLightSpace = mul(position, lightViewProj);
	
	// get how much the pixel is in light 
	float lightPercent = GetShadowLightFraction(Texture[0], SampleType[1], positionLightSpace, 0.00001);
	if (lightPercent == 0)
		return (0,0,0,0);
										  
	// sample from textures, multiply texturecolor with light percent to get blured shadow edges		
	float4 textureColor = diffuse * lightPercent;
			 
	// get light intensity			 
    float lightIntensity = saturate(dot(normal.xyz, lightDirectionDir)); 
	
	// get color based on texture, intensity and color of light
    float4 color = textureColor * lightIntensity * lightColorDir; 
	
	float4 specular = float4(0,0,0,0);		
	if (lightIntensity > 0)
		specular = GetSpecularColorPhong(cameraPosition, position.rgb, lightDirectionDir, lightColorDir, lightIntensity, normal, specularMap);
			
	return color + specular; 
}

float4 GetPointColor(float4 position, float4 diffuse, float4 normal, float4 specularMap)
{
	float4 finalColor = float4(0,0,0,1);
	int numLights     = pointLights[0].numLights;
	
	for (int i =0; i < numLights; i++)
	{				
		float3 lightDir   = -normalize(position - pointLights[i].position);	
		float dst         = length(position - pointLights[i].position);				
		float fallOff     = 1 - ((dst / pointLights[i].radius) * 0.5);
		float attuniation = 1 / (pointLights[i].attConstant + pointLights[i].attLinear * dst + pointLights[i].attExponential * dst * dst);
		attuniation       = (attuniation * pointLights[i].intensity) * fallOff;
		
		if (fallOff >= 0)
		{					
			float lightIntensity = saturate(dot(normal.xyz, lightDir)); 
			float4 color         = diffuse * lightIntensity * float4(pointLights[i].color,1); 
			float4 specular      = float4(0,0,0,0);	
			
			if(lightIntensity > 0 && dst < pointLights[i].radius)
			{						
				float3 vertexToCamera   = normalize(cameraPosition.xyz - position.xyz);
				float3 reflection       = normalize(2 * lightIntensity * normal - lightDir);		
				float specularIntensity = pow(saturate(dot(reflection, vertexToCamera)), specularMap.a * 255.0);
				specular.rgb            = (pointLights[i].color * specularIntensity) * specularMap.rgb;
			} 
						
			color += specular;	
			finalColor += (color * attuniation);
		}
	}
      
    return finalColor;
}

float4 Main(PixelInputType input) : SV_TARGET
{            
	// alpha channel is flagged as 0 in the diffusemap if this is an emissive pixel
	// dont do any lightning calculations and only return the emissive
	// color that is stored in the diffuse map
	float4 diffuse  = Texture[3].Load(int3(input.position.xy,0));
	if (diffuse.a == 0)	
		return diffuse;
		
	float4 position = Texture[1].Load(int3(input.position.xy,0));
	float4 normal   = Texture[2].Load(int3(input.position.xy,0));
	float4 specular = Texture[4].Load(int3(input.position.xy,0));		
		
	float4 finalColor = float4(0,0,0,1);

	float4 ambientColor     = GetAmbientColor(diffuse);
	float4 directionalColor = GetDirectionalColor(position, diffuse, normal, specular);
	float4 pointColor       = GetPointColor(position, diffuse, normal, specular);
	
	finalColor = ambientColor + directionalColor + pointColor;
	
    return finalColor;	   
}






























