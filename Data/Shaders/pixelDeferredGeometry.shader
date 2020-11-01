#include "CommonPixel.shader"
Texture2D Textures[5];	// diffuse, normal, metalic, rougness, emissive
SamplerState SampleType[6]; // wrapTrilinear, clampTrilinear, wrapBilinear, clampBililinear, wrapAnisotropic, clampAnisotropic	

float4 u_cameraPosition;
int    u_hasHeightmap; 	
float  u_heightScale;
float  u_metalic;
float  u_rougness;
	
struct PixelInputType
{
    float4 position      : SV_POSITION;
    float2 texCoord      : TEXCOORD0;
	float3 normal        : NORMAL;
	float3 tangent       : TANGENT;
	float3 binormal      : BINORMAL;
	float4 worldPosition : TEXCOORD1;
	float4 color         : COLOR;
};

struct Output
{
	float4 position : SV_TARGET0;
	float4 normal   : SV_TARGET1;
	float4 diffuse  : SV_TARGET2;
	float4 pbr      : SV_TARGET3;
};

Output Main(PixelInputType input) 
{
    Output output;
	
	input.normal   = normalize(input.normal);
	input.binormal = normalize(input.binormal);
	input.tangent  = normalize(input.tangent);
	
	float3x3 TBNMatrix = float3x3(input.tangent, input.binormal, input.normal);	
	
	if (u_hasHeightmap == 1)
		input.texCoord += GetPOMOffset(Textures[1], SampleType[0], input.normal, input.tangent, input.worldPosition, u_cameraPosition.xyz, input.texCoord, u_heightScale);
	
	float4 textureColor = Textures[0].Sample(SampleType[0], input.texCoord) * input.color;
	
	// alpha clip pixel if it contains transparancy
	if (textureColor.a < 0.5) discard;
	
	float4 normalMap    = Textures[1].Sample(SampleType[0], input.texCoord);
	float4 metalicMap   = Textures[2].Sample(SampleType[0], input.texCoord);
	float4 rougnessMap  = Textures[3].Sample(SampleType[0], input.texCoord);
	float4 emissiveMap  = Textures[4].Sample(SampleType[0], input.texCoord);
			
	normalMap         = (normalMap * 2.0) -1.0;
	float3 bumpNormal = normalize(mul(normalMap, TBNMatrix));   	
			    		  					   		    				
	output.position = input.worldPosition;
	output.normal   = float4(bumpNormal.xyz,1);
	output.diffuse  = float4(textureColor.rgb, 1);
	output.pbr.r    = metalicMap.x  * u_metalic;
	output.pbr.g    = rougnessMap.x * u_rougness;
	
	output.pbr.a = 1;
	
	// flag the alpha channel with 0 so the deferred ligtning pass can check for this and
	// know if it should perfom lightning calculations or not on this pixel
	if (emissiveMap.r + emissiveMap.g + emissiveMap.b > 0.1)
		output.diffuse = float4(emissiveMap.rgb * 1.1, 0);
			
	return output;    
}