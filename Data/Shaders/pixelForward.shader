#include "CommonPixel.shader"
Texture2D Texture[5];	    // diffuse, normal, specular, emissive, shadowmap
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
	int    hasHeightmap; 	
	float  heightScale;
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
	float3 cameraPos          : TEXCOORD4;
};

float3 GetBaseColor(PixelInputType input, float3 textureColor)
{
	return (textureColor * input.vertexColor) * ambientColor;
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
		// get light direction, distance from light	and calculate attuniation
		float3 lightDir   = -normalize(input.worldPos - pointLights[i].position);	
		float dst         = length(input.worldPos - pointLights[i].position);
		float fallOff     = 1 - ((dst / pointLights[i].radius) * 0.5);	// make the falloff 50% slower then the radius to give more control of result depending of attuniation settings
		float attuniation = 1 / (pointLights[i].attConstant + pointLights[i].attLinear * dst + pointLights[i].attExponential * dst * dst);
		attuniation       = (attuniation * pointLights[i].intensity) * fallOff;
		
		if (fallOff >= 0)
		{						
			// get intensity from the normal from normalmap and lightdirection
			float lightIntensity = saturate(dot(bumpNormal, lightDir)); 
			float3 color         = textureColor * input.vertexColor.rgb * lightIntensity * pointLights[i].color; 						
			float3 specular      = float3(0,0,0);
			
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

float2 GetTexCoordOffset(PixelInputType input, float3x3 TBNMatrix)
{
	// calculate tex coords for POM here
	float3x3 toTangentSpace = transpose(TBNMatrix);
	
	float3 viewDirectionW = normalize(-input.vertexToCamera);
	float3 viewDirectionT = mul(viewDirectionW, toTangentSpace);
	
	return float2(0.0, 0.0);
}

float4 Main(PixelInputType input) : SV_TARGET
{						
	// if the alpha channel of the emissivemap is not zero it means that this is 
	// an emissive pixel and should not recive lightning
	float4 emissiveMap = Texture[3].Sample(SampleType[0],input.texCoord);
	if (emissiveMap.r + emissiveMap.g + emissiveMap.b > 0.1)
		return float4(emissiveMap.rgb * 1.1, 1);
	
	input.normal   = normalize(input.normal);
	input.binormal = normalize(input.binormal);
	input.tangent  = normalize(input.tangent);
	
	float3x3 TBNMatrix = float3x3(input.tangent, input.binormal, input.normal);
	
	if (hasHeightmap == 1)
		input.texCoord += GetPOMOffset(Texture[1], SampleType[0], input.normal, input.tangent, input.worldPos, input.cameraPos, input.texCoord, heightScale);
	
	float4 textureColor = Texture[0].Sample(SampleType[0], input.texCoord);;		
	float4 normalMap    = Texture[1].Sample(SampleType[0], input.texCoord);
	float4 specularMap  = Texture[2].Sample(SampleType[0], input.texCoord);
	
	// convert normalmap sample to range -1 to 1 and convert to worldspace
	normalMap         = (normalMap * 2.0) -1.0;	
	float3 bumpNormal = normalize(mul(normalMap, TBNMatrix));  
	
	float3 baseColor        = GetBaseColor(input, textureColor.rgb);
	float3 directionalColor = GetDirectionalColor(input, textureColor, bumpNormal, specularMap);
	float3 pointColor       = GetPointColor(input, textureColor.rgb, bumpNormal, specularMap);
	
	return float4(baseColor + directionalColor + pointColor, textureColor.a);
}






