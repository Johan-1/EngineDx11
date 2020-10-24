#include "CommonPixel.shader"

Texture2D Texture[10];	    // normal, specular, DUDVMap, shadowMap, reflectionMap, refractionMap, refractionDepth, foam, foamNoise, normal2
SamplerState SampleType[2]; // wrap,clamp 	

static float nearPlane = 0.1;
static float farPlane  = 5000.0;
static float darkenOcludedPercent = 0.4;

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

cbuffer WaterBuffer : register(b2)
{
	float4 waterColorTint;
	float highlightStrength;  
	float tintFraction;
	float waterDistortionDamping;
	float reflectivePower;
	float fadeToDepth;
	float normalScrollStrength1;
	float normalScrollStrength2;
	bool  applyFoam;
	float foamToDepth;
	float foamDistortStrength;
	float foamScrollStrength;
	float foamTileMultiplier;
	float foamNoiseTileMultiplier;
    float foamDistortionDamping;
}

struct PixelInputType
{
    float4 position            : SV_POSITION;
    float2 texCoord            : TEXCOORD0;
	float3 normal              : NORMAL;
	float3 tangent             : TANGENT;
	float3 binormal            : BINORMAL;
	float3 vertexToCamera      : TEXCOORD1;
	float4 positionLightSpace  : TEXCOORD2;
	float3 worldPos            : TEXCOORD3;
	float4 vertexColor         : COLOR;
	float4 reflectionPosition  : TEXCOORD4;
	float4 positionCameraSpace : TEXCOORD5;
	float2 texCoordOffset      : TEXCOORD6;
};

float3 GetDirectionalColor(PixelInputType input, float3 baseColor, float3 bumpNormal, float4 specularMap)
{			    		  					   		
    float lightIntensity = saturate(dot(bumpNormal.xyz, dirLightDirection)); 
    float3 color         = (baseColor * dirDiffuseColor.rgb) * lightIntensity; 
	float3 specular      = float3(0,0,0);
	
	// add specular effect if the light is affecting the pixel
	if (lightIntensity > 0)
	{					
		float3 vtc              = normalize(input.vertexToCamera);
		float3 reflection       = normalize(2 * lightIntensity * bumpNormal - dirLightDirection);		
		float specularIntensity = pow(saturate(dot(reflection, vtc)), specularMap.a * 255.0);
		specular                = (dirDiffuseColor * specularIntensity) * specularMap.rgb ;	
	} 
			
	return color + ((dirDiffuseColor * specular) * highlightStrength); 
}

float3 GetPointColor(PixelInputType input, float3 baseColor, float3 bumpNormal, float4 specularMap)
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
			// get intensity from the normal from normalmap and lightdirection
			float lightIntensity = saturate(dot(bumpNormal, lightDir)); 
			float3 color         = (baseColor * pointLights[i].color.rgb) * lightIntensity; 
			float3 specular      = float3(0,0,0);	
			
			if (lightIntensity > 0 && dst < pointLights[i].radius)
			{		
				float3 vtc              = normalize(input.vertexToCamera);
				float3 reflection       = normalize(2 * lightIntensity * bumpNormal - lightDir);		
				float specularIntensity = pow(saturate(dot(reflection, vtc)), specularMap.a * 255.0);
				specular.rgb            = (pointLights[i].color.rgb * specularIntensity) * specularMap.rgb;	
			} 

			color      += (specular * highlightStrength);	
			finalColor += (color * attuniation);
		}
	}
     
    return finalColor;
}

float4 GetReflectionColor(PixelInputType input, float2 distortion)
{
	return Texture[4].Sample(SampleType[1], clamp(GetProjectiveTexCoords(input.reflectionPosition) + distortion, 0.001, 0.999));
}

float4 GetRefractionColor(PixelInputType input, float2 distortion)
{
	return Texture[5].Sample(SampleType[1], clamp(GetProjectiveTexCoords(input.positionCameraSpace) + distortion, 0.001, 0.999));
}

float GetWaterDepth(PixelInputType input)
{	
	float bottomDepth  = Texture[6].Sample(SampleType[1], GetProjectiveTexCoords(input.positionCameraSpace)).r;

	// convert depth values to linear distances
	float bottomDistance   = ConvertToLinearDepth(nearPlane, farPlane, bottomDepth);
	float surfaceDistance  = ConvertToLinearDepth(nearPlane, farPlane, input.position.z);

	return bottomDistance - surfaceDistance;
}

float4 Main(PixelInputType input) : SV_TARGET
{
	// sample the UVDV map to get refraction amount
	float2 distortionMap = (Texture[2].Sample(SampleType[0], input.texCoord + input.texCoordOffset).rg * 2.0 -1.0);
	float2 waterDistortStrength = distortionMap * waterDistortionDamping;

	// add the distortion amount to the regular texCoords for sampling normal and specular maps
	float2 distortedTexCoords = input.texCoord + waterDistortStrength;
	float4 normalMap1  = Texture[0].Sample(SampleType[0], distortedTexCoords + (input.texCoordOffset * normalScrollStrength1));
	float4 normalMap2  = Texture[9].Sample(SampleType[0], distortedTexCoords + (-input.texCoordOffset * normalScrollStrength2));
	float4 specularMap = Texture[1].Sample(SampleType[0], distortedTexCoords + (input.texCoordOffset * normalScrollStrength1));
	
	// convert normalmap sample to range -1 to 1 and convert to worldspace
	normalMap1                  = (normalMap1 * 2.0) -1.0;
	normalMap2                  = (normalMap2 * 2.0) -1.0;
	float3x3 tangentSpaceMatrix = float3x3(input.tangent,input.binormal,input.normal);	
	float3 bumpNormal1          = normalize(mul(normalMap1, tangentSpaceMatrix)); 
    float3 bumpNormal2          = normalize(mul(normalMap2, tangentSpaceMatrix)); 	
	float3 bumpNormalFinal      = lerp(bumpNormal1, bumpNormal2, 0.5);

	// get the colors from the projected reflection and distortion maps
	float4 reflectionColor = GetReflectionColor(input, waterDistortStrength);
	float4 refractionColor = GetRefractionColor(input, waterDistortStrength);

	// get the amount of reflective/transparency based on the fresnel effect
	float fresnelDot = saturate(dot(float3(0, 1, 0), normalize(input.vertexToCamera))); 
	fresnelDot = pow(fresnelDot, reflectivePower);

	// set the mix of reflection color and refraction color
	float4 colorMix = lerp(reflectionColor, refractionColor, fresnelDot);
	colorMix = lerp (colorMix, waterColorTint, tintFraction);

	// get how much this pixel is ocluded
	float lightPercent = GetShadowLightFraction(Texture[3], SampleType[1], input.positionLightSpace, 0.00001);
	
	// get the minimum color value if the pixel is 100% ocluded
	// and the leftover color that will represent if the pixel is fully luminated
	float3 ocludedColor   = colorMix * (1 - darkenOcludedPercent);
	float3 fullLightColor = (colorMix * darkenOcludedPercent) * lightPercent;
	
	// get the specular highlights for the water surface
	float3 directionalColor = GetDirectionalColor(input, fullLightColor, bumpNormalFinal, specularMap) * lightPercent;
	float3 pointColor       = GetPointColor(input, fullLightColor, bumpNormalFinal, specularMap);
	
	float4 finalColor = float4(0,0,0,1);
	finalColor.rgb = ocludedColor + directionalColor + pointColor; 
		
	float waterDepth = GetWaterDepth(input);

	// add foam to certain water depth
	if (applyFoam)
	{
		// get how much the surface should use the calculated water color or the foam color
		float waterFraction = InverseLerp(0.0, foamToDepth, waterDepth);
		
		// get tex coords to use for foam texture and noise texture
		float2 foamDistortStrength = input.texCoord + (distortionMap * foamDistortionDamping);
		float2 foamTexCoords  = ((input.texCoord * foamTileMultiplier)      + foamDistortStrength) + (input.texCoordOffset * foamScrollStrength);
		float2 noiseTexCoords = ((input.texCoord * foamNoiseTileMultiplier) + foamDistortStrength) + (input.texCoordOffset * foamScrollStrength);
		
		// sample the foam texture
		float4 foamColor = Texture[7].Sample(SampleType[0], foamTexCoords);
		
		// sample the noise texture and flip the white and black colors
		// this make black in texture represent no foam and white will represent no modification to foam at all
		float foamNoiseReduce = InverseLerp(1.0, 0.0, Texture[8].Sample(SampleType[0], noiseTexCoords).r);
		
		// add the noise to amount of the water color that will be used		
		waterFraction = saturate(waterFraction += foamNoiseReduce);
		
		return lerp(foamColor, finalColor, waterFraction);
	}	

	// get the linear depth of the water and fade water edges starting 
	// at fully transparent and ending with 100 % opaque at set depth value
	float fadeStrength = InverseLerp(0.0, fadeToDepth, waterDepth);	
	finalColor.a = lerp(0.0, 1.0, fadeStrength);

	return finalColor;
}






