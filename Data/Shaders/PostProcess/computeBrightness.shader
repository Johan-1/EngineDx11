
Texture2D<float4>   inputTex : register (t0);
RWTexture2D<float4> writeTex : register (u0);

// 32 * 32 * 1 threads per group
// set the dispach groups accordingly 
// to get one thread per texel
[numthreads(32, 32, 1)]
void Main( uint3 GroupID : SV_GroupID, uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex )
{
	// Offset the texture location to the first sample location
	int3 texturelocation = DispatchThreadID;

	float4 color = inputTex.Load(texturelocation);

	float brightness = (color.r * 0.2126) + (color.g * 0.7152) + (color.b * 0.0722);

	writeTex[DispatchThreadID.xy] = float4(color.rgb * brightness, 1);
}