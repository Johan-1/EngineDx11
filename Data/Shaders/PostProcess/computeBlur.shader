Texture2D<float4>   inputTex : register (t0);
RWTexture2D<float4> writeTex : register (u0);

uniform float u_screenWidth;
uniform float u_screenHeight;

static float weights[11] =
{
	0.0093,
	0.028002,
	0.065984,
	0.121703,
	0.175713,
	0.198596,
	0.175713,
	0.121703,
	0.065984,
	0.028002,
	0.0093
};

// 32 * 32 * 1 threads per group
// set the dispach groups accordingly 
// to get one thread per texel
[numthreads(32, 32, 1)]
void MainH( uint3 GroupID : SV_GroupID, uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex )
{	
	float4 color = float4(0, 0, 0, 1);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth - 5, u_screenHeight, 1), DispatchThreadID + float3(-5, 0, 0))) * weights[0]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth - 5, u_screenHeight, 1), DispatchThreadID + float3(-4, 0, 0))) * weights[1]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth - 5, u_screenHeight, 1), DispatchThreadID + float3(-3, 0, 0))) * weights[2]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth - 5, u_screenHeight, 1), DispatchThreadID + float3(-2, 0, 0))) * weights[3]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth - 5, u_screenHeight, 1), DispatchThreadID + float3(-1, 0, 0))) * weights[4]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth - 5, u_screenHeight, 1), DispatchThreadID + float3( 0, 0, 0))) * weights[5]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth - 5, u_screenHeight, 1), DispatchThreadID + float3( 1, 0, 0))) * weights[6]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth - 5, u_screenHeight, 1), DispatchThreadID + float3( 2, 0, 0))) * weights[7]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth - 5, u_screenHeight, 1), DispatchThreadID + float3( 3, 0, 0))) * weights[8]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth - 5, u_screenHeight, 1), DispatchThreadID + float3( 4, 0, 0))) * weights[9]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth - 5, u_screenHeight, 1), DispatchThreadID + float3( 5, 0, 0))) * weights[10]);

	writeTex[DispatchThreadID.xy] = float4(color.rgb, 1);
}

// 32 * 32 * 1 threads per group
// set the dispach groups accordingly 
// to get one thread per texel
[numthreads(32, 32, 1)]
void MainV( uint3 GroupID : SV_GroupID, uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex )
{
	float4 color = float4(0, 0, 0, 1);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth, u_screenHeight - 5, 1), DispatchThreadID + float3(0, -5, 0))) * weights[0]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth, u_screenHeight - 5, 1), DispatchThreadID + float3(0, -4, 0))) * weights[1]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth, u_screenHeight - 5, 1), DispatchThreadID + float3(0, -3, 0))) * weights[2]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth, u_screenHeight - 5, 1), DispatchThreadID + float3(0, -2, 0))) * weights[3]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth, u_screenHeight - 5, 1), DispatchThreadID + float3(0, -1, 0))) * weights[4]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth, u_screenHeight - 5, 1), DispatchThreadID + float3(0,  0, 0))) * weights[5]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth, u_screenHeight - 5, 1), DispatchThreadID + float3(0,  1, 0))) * weights[6]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth, u_screenHeight - 5, 1), DispatchThreadID + float3(0,  2, 0))) * weights[7]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth, u_screenHeight - 5, 1), DispatchThreadID + float3(0,  3, 0))) * weights[8]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth, u_screenHeight - 5, 1), DispatchThreadID + float3(0,  4, 0))) * weights[9]);
	color += (inputTex.Load(clamp(int3(0,0,0), int3(u_screenWidth, u_screenHeight - 5, 1), DispatchThreadID + float3(0,  5, 0))) * weights[10]);

	writeTex[DispatchThreadID.xy] = float4(color.rgb, 1);
}