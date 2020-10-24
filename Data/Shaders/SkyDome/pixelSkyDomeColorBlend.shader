 uniform float4 u_bottomColor;
 uniform float4 u_midColor;
 uniform float4 u_topColor;
 
struct PixelInputType
{
    float4 position : SV_POSITION;
	float  height   : TEXCOORD0;
};

float GetColorFraction(float a, float b, float t)
{
	return saturate((t - a) / (b - a));
}

float4 Main(PixelInputType input) : SV_TARGET
{
	// convert hight of sphere from min -0.5 - max 0.5 to min 0 - max 100
	float heightPercent = (input.height + 0.5) * 100.0;
	
	float4 bottomMid = lerp(float4(u_bottomColor.rgb, 1.0), float4(u_midColor.rgb, 1.0), GetColorFraction(u_bottomColor.a, u_midColor.a, heightPercent));
	float4 midTop    = lerp(float4(u_midColor.rgb, 1.0),    float4(u_topColor.rgb, 1.0), GetColorFraction(u_midColor.a, u_topColor.a, heightPercent));
	float4 final     = lerp(float4(bottomMid.rgb, 1.0),      float4(midTop.rgb, 1.0),    GetColorFraction(0, 100, heightPercent));
	
	return final;
}