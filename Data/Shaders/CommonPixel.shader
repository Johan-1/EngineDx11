static int heightMinSampleCount = 8;
static int heightMaxSampleCount = 64;
static float PI = 3.14159265359;

float2 GetProjectiveTexCoords(float4 clipPosition)
{
	float2 result;
	result.x =  clipPosition.x / clipPosition.w / 2.0f + 0.5f;
    result.y = -clipPosition.y / clipPosition.w / 2.0f + 0.5f;
	
	return result;
}

float InverseLerp(float a, float b, float t)
{
	return saturate((t - a) / (b - a));
}

float ConvertToLinearDepth(float nearPlane, float farPlane, float depth)
{
	return 2.0 * nearPlane * farPlane / (farPlane + nearPlane - (2.0 * depth - 1.0) * (farPlane - nearPlane));
}

float4 GetSpecularColorPhong(float3 cameraPosition, float3 worldPosition, float3 lightDirection, float4 lightColor, float lightIntensity, float3 normal, float4 specularMap)
{		
	float3 vertexToCamera   = normalize(cameraPosition - worldPosition);				
	float3 reflection       = normalize(2 * lightIntensity * normal - lightDirection);		
	float specularIntensity = pow(saturate(dot(reflection, vertexToCamera)), specularMap.a * 255.0);
	
	return (lightColor * specularIntensity) * specularMap;			
}

float4 GetSpecularColorPhong(float3 vertexToCamera, float3 lightDirection, float4 lightColor, float lightIntensity, float3 normal, float4 specularMap)
{
		float3 reflection       = normalize(2 * lightIntensity * normal - lightDirection);		
		float specularIntensity = pow(saturate(dot(reflection, vertexToCamera)), specularMap.a * 255.0);
		
		return  (lightColor * specularIntensity) * specularMap;		
}

float GetShadowLightFraction(Texture2D shadowMap, SamplerState s, float4 posLightClipSpace, float bias)
{
	// if pixel is outside the range of the shadow map we will fallback that it is just lit
	float numInLight   = 9;
	
	// get the projected texture coordinates based on the position in eye of the light
	float2 smTexCoord = GetProjectiveTexCoords(posLightClipSpace);
	
	if((saturate(smTexCoord.x) == smTexCoord.x) && (saturate(smTexCoord.y) == smTexCoord.y))
	{
		// if whithin the shadow map initialize num in light to 0
		numInLight = 0;
		
		float neighbour = 1 / 8192.0;	// hardcoded for now, dont forget to change if depthmap res change	
		float depthMapValues[9];
		
		depthMapValues[0] = shadowMap.Sample(s, float2(smTexCoord.x, smTexCoord.y)).r;                         // middle
		depthMapValues[1] = shadowMap.Sample(s, float2(smTexCoord.x - neighbour, smTexCoord.y)).r;             // left
		depthMapValues[2] = shadowMap.Sample(s, float2(smTexCoord.x - neighbour, smTexCoord.y - neighbour)).r; // top left
		depthMapValues[3] = shadowMap.Sample(s, float2(smTexCoord.x, smTexCoord.y - neighbour)).r;             // top
		depthMapValues[4] = shadowMap.Sample(s, float2(smTexCoord.x + neighbour, smTexCoord.y - neighbour)).r; // top right
		depthMapValues[5] = shadowMap.Sample(s, float2(smTexCoord.x + neighbour, smTexCoord.y)).r;             // right
		depthMapValues[6] = shadowMap.Sample(s, float2(smTexCoord.x + neighbour, smTexCoord.y + neighbour)).r; // bottom right
		depthMapValues[7] = shadowMap.Sample(s, float2(smTexCoord.x, smTexCoord.y + neighbour)).r;             // bottom 
		depthMapValues[8] = shadowMap.Sample(s, float2(smTexCoord.x - neighbour, smTexCoord.y + neighbour)).r; // bottom left
		
		float lightDepthValue = (posLightClipSpace.z / posLightClipSpace.w) - bias;
		
		for(int i =0; i < 9; i++)		
			if( lightDepthValue <= depthMapValues[i])
				numInLight++;		
	}
	
	return numInLight / 9.0;
}

float2 GetPOMOffset(Texture2D normalMap, SamplerState samplerType, float3 normalW, float3 tangentW, float3 positionW, float3 cameraPositionW, float2 texCoords, float heightScale)
{	
	// create the orthonormal basis
	float3 T = normalize(tangentW - dot(tangentW, normalW) * normalW);
	float3 B = cross(normalW, T);
	
	// get matrix for transforming from world to tangent space
	float3x3 toTangentSpace = transpose(float3x3(T, B, normalW)); 
	
	// get the view direction of camera in both world and tangent space
	float3 viewDirectionW = normalize(positionW - cameraPositionW);
	float3 viewDirectionT = mul(viewDirectionW, toTangentSpace);
	
	// get the max uv offset  
	// the height scale is a tweakable value for how intense the parallax will be
	// the correct way of getting the heightScale is to convert
	// one world unit of mesh to one unit in texture space [0-1] relative to the size of the mesh and tiling of texture
    // but this gives more control of the effect
	float2 maxParallelOffset = -viewDirectionT.xy * heightScale / viewDirectionT.z;
	
	// get amount of samples we will do based on the dot between inversed view and surface normal
	// min max is tweakable values here, heigher == better precision but more expensive
	int sampleCount = (int)lerp(heightMaxSampleCount, heightMinSampleCount, dot(-viewDirectionW, normalW));
	
	// get the value of each step into the texture 
	float zStep = 1.0 / (float)sampleCount;
	
	// get how much one step is in texCoords based on the max offset and zStep
	float2 texStep = maxParallelOffset * zStep;
	
	// this must be done to be abble to sample the correct mip maps in a loop
	float2 dx = ddx(texCoords);
	float2 dy = ddy(texCoords);
	
	int i                 = 0;           // start index count of how many samples we have done
    float2 currTexOffset  = 0;           // the start texture offset of the first iteration
    float2 prevTexOffset  = 0;           // the tex offset we had on the last failed intersection test
    float2 finalTexOffset = 0;           // the end result texture offset
    float currRayZ        = 1.0 - zStep; // the length of the current ray deep into the texture, v starts at 1 therefore we have to decrement the ray value to go deeper into the texture
    float prevRayZ        = 1.0;         // the length of the ray from the last failed intersection test
    float currHeight      = 0.0;         // the current sampled height from the heightmap
    float prevHeight      = 0.0;         // the heightmap value from last sample
	
	// V [0,1]       [1,1]
	// |------------------
	// |------------------
	// |------------------
	// |------------------
	// |------------------
	// |[0,0]        [1,0]
	// -------------------U
	
	while (i < sampleCount)
	{
		// sample the height from our current texture coords
		currHeight = normalMap.SampleGrad(samplerType, texCoords + currTexOffset, dx, dy).a;
		
		// if the sampled height is higher then the current length of ray 
		// we have found the height intersection
		if (currHeight > currRayZ)
		{			
			// get the final texture offset
			float t = (prevHeight - prevRayZ) / (prevHeight - currHeight + currRayZ - prevRayZ);
			finalTexOffset = prevTexOffset + t * texStep;
			
			// exit loop
			i = sampleCount;
		}
		else
		{
			// increment step values
			i++;
			prevTexOffset  = currTexOffset;
			prevRayZ       = currRayZ;
			prevHeight     = currHeight;
			currTexOffset += texStep;
			
			// go deeper into the texture by step length 
			currRayZ -= zStep;
		}
	}
	
	return finalTexOffset;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float3 GetLightRadiance(float4 position, float4 baseColor, float4 normal, float4 pbr, float3 cameraPosition, float3 lightDirection, float3 lightColor)
{
	float3 radiance = lightColor.rgb;

	float3 N = normalize(normal.xyz);
	float3 V = normalize(cameraPosition.xyz - position.xyz);
	float3 L = normalize(lightDirection);
	float3 H = normalize(V + L);

	float3 F0 = lerp(float3(0.04, 0.04, 0.04), baseColor.rgb, float3(pbr.r, pbr.r, pbr.r));

	// cook-torrance brdf
	float NDF = DistributionGGX(N, H, pbr.g);
	float G   = GeometrySmith(N, V, L, pbr.g);
	float3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

	float3 kS = F;
	float3 kD = float3(1.0, 1.0, 1.0) - kS;
	kD *= 1.0 - pbr.r;

	float3 numerator = NDF * G * F;
	float  denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	float3 specular = numerator / max(denominator, 0.001);

	float NdotL = max(dot(N, L), 0.0);

	return (kD * baseColor.rgb / PI + specular) * radiance * NdotL;
}

