Texture2D    Texture[1];    // ScenemapSDR
SamplerState SampleType[6]; // wrapTrilinear, clampTrilinear, wrapBilinear, clampBililinear, wrapAnisotropic, clampAnisotropic

uniform int   u_applyFXAA;
uniform float u_screenWidth;
uniform float u_screenHeight;

#define EDGE_THRESHOLD_MIN 0.0312
#define EDGE_THRESHOLD_MAX 0.125
#define ITERATIONS 24
#define SUBPIXEL_QUALITY 0.75
#define QUALITY(q) (q < 12 ? 1.0 : 2.0)

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;		
};

float3 RGBToLuma(float3 rgb)
{
	return sqrt(dot(rgb, float3(0.299, 0.587, 0.114)));
}

// implemented from this blogpost that goes into detail on how this AA technique works
// the blogpost is in its turn implemeted from the FXAA implementation created by Timothy Lottes at NVIDIA
// http://blog.simonrodriguez.fr/articles/30-07-2016_implementing_fxaa.html
float3 GetFXAAColor(PixelInputType input, float4 baseColor)
{
	float2 inverseScreenSize;
	inverseScreenSize.x = 1.0 / u_screenWidth;
	inverseScreenSize.y = 1.0 / u_screenHeight;
	
	// get the luma of the centerpixel and its four neighbours
	float lumaCenter = RGBToLuma(Texture[0].Sample(SampleType[0], input.texCoord).rgb);
	float lumaDown   = RGBToLuma(Texture[0].Sample(SampleType[0], input.texCoord + float2(0,                    -inverseScreenSize.y)).rgb);     
	float lumaUp     = RGBToLuma(Texture[0].Sample(SampleType[0], input.texCoord + float2(0,                    inverseScreenSize.y)).rgb);      
	float lumaLeft   = RGBToLuma(Texture[0].Sample(SampleType[0], input.texCoord + float2(-inverseScreenSize.x, 0)).rgb);     
	float lumaRight  = RGBToLuma(Texture[0].Sample(SampleType[0], input.texCoord + float2(inverseScreenSize.x,  0)).rgb);      
	
	// get the maximum and minimum luma around the center luma and get the difference delta
	float lumaMin   = min(lumaCenter, min(min(lumaDown, lumaUp),min(lumaLeft, lumaRight)));
	float lumaMax   = max(lumaCenter, max(max(lumaDown, lumaUp),max(lumaLeft, lumaRight)));
	float lumaRange = lumaMax - lumaMin;
	
	// If the luma delta is less then our edge threshold we likely are not on a edge 
	// or the edge color and the pixel color behind the edge is similar in color so no AA is needed
	if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX))			
		return baseColor.rgb;
		
	// get the 4 corner lumas around the center pixel
	float lumaDownLeft  = RGBToLuma(Texture[0].Sample(SampleType[0], input.texCoord + float2(-inverseScreenSize.x, -inverseScreenSize.y)).rgb);
	float lumaUpRight   = RGBToLuma(Texture[0].Sample(SampleType[0], input.texCoord + float2(inverseScreenSize.x, inverseScreenSize.y)).rgb);
	float lumaUpLeft    = RGBToLuma(Texture[0].Sample(SampleType[0], input.texCoord + float2(-inverseScreenSize.x, inverseScreenSize.y)).rgb);
	float lumaDownRight = RGBToLuma(Texture[0].Sample(SampleType[0], input.texCoord + float2(inverseScreenSize.x, - inverseScreenSize.y)).rgb);
	
	// Combine the orientation lumas
	float lumaDownUp       = lumaDown      + lumaUp;
	float lumaLeftRight    = lumaLeft      + lumaRight;
	float lumaLeftCorners  = lumaDownLeft  + lumaUpLeft;
	float lumaDownCorners  = lumaDownLeft  + lumaDownRight;
	float lumaRightCorners = lumaDownRight + lumaUpRight;
	float lumaUpCorners    = lumaUpRight   + lumaUpLeft;
	
	// Compute an estimation of the gradient along the horizontal and vertical axis to decide the orientation of this edge
	float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + abs(-2.0 * lumaCenter + lumaDownUp)    * 2.0 + abs(-2.0 * lumaRight + lumaRightCorners);
	float edgeVertical   = abs(-2.0 * lumaUp   + lumaUpCorners)	  + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0 + abs(-2.0 * lumaDown  + lumaDownCorners);
	
	// get orientation of edge and get stepsize of one pixel
	bool isHorizontal = edgeHorizontal >= edgeVertical;
	float stepLength  = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;
	
	// Select the two neighboring texel lumas in the perpendicular direction to the local edge
	// and get the gradients of thier directions
	float luma1     = isHorizontal ? lumaDown : lumaLeft;
	float luma2     = isHorizontal ? lumaUp   : lumaRight;
	float gradient1 = luma1 - lumaCenter;
	float gradient2 = luma2 - lumaCenter;
	
	// get the direction that is steepest and get the gradient in that same direction
	bool firstIsSteepest = abs(gradient1) >= abs(gradient2);
	float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));
	
	// Average luma in calculated direction
	float lumaLocalAverage = 0.0;
	if (firstIsSteepest)
	{
		// invert stepLength if we are going in left or down direction
		stepLength       = - stepLength;
		lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
	} 
	else 
	{
		lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
	}
	
	// Shift UV in the correct direction by half a pixel.
	float2 currentUv = input.texCoord;
	if (isHorizontal) currentUv.y += stepLength * 0.5;
	else              currentUv.x += stepLength * 0.5;
		
	// get UV offset for each step depending of direction
	float2 offset = isHorizontal ? float2(inverseScreenSize.x, 0.0) : float2(0.0, inverseScreenSize.y);
	
	// get the first uv offsets in both positive and negative along our direction
	float2 uv1 = currentUv - offset * QUALITY(0);
	float2 uv2 = currentUv + offset * QUALITY(0);
	
	float lumaEnd1   = 0.0;
	float lumaEnd2   = 0.0;	
	bool reached1    = false;
	bool reached2    = false;
			
	for (int i = 0; i < ITERATIONS; i++)
	{
		// get luma in negative direction
		if (!reached1)
		{
			lumaEnd1 = RGBToLuma(Texture[0].Sample(SampleType[0], uv1).rgb);
			lumaEnd1 = lumaEnd1 - lumaLocalAverage;
		}
		
		// get luma in positive direction
		if (!reached2)
		{
			lumaEnd2 = RGBToLuma(Texture[0].Sample(SampleType[0], uv2).rgb);
			lumaEnd2 = lumaEnd2 - lumaLocalAverage;
		}
		
		// If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge
		reached1 = abs(lumaEnd1) >= gradientScaled;
		reached2 = abs(lumaEnd2) >= gradientScaled;
		
		// If the side is not reached, we continue to explore in this directions
		// quality is set to increase step size if we surpass half of our max iterations
		if (!reached1) uv1 -= offset * QUALITY(i);		
		if (!reached2) uv2 += offset * QUALITY(i);
				
		// break out if we have reached desired lumas
		if (reached1 && reached2) break;
	}

	// compute the distances to each side edge of the edge
	float distance1 = isHorizontal ? (input.texCoord.x - uv1.x) : (input.texCoord.y - uv1.y);
	float distance2 = isHorizontal ? (uv2.x - input.texCoord.x) : (uv2.y - input.texCoord.y);
	
	// in which direction is the side of the edge closer
	bool isDirection1   = distance1 < distance2;
	float distanceFinal = min(distance1, distance2);
	
	// Thickness of the edge
	float edgeThickness = (distance1 + distance2);
	
	// Is the luma at center smaller than the local average
	bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
	
	// If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive
	bool correctVariation1 = (lumaEnd1 < 0.0) != isLumaCenterSmaller;
	bool correctVariation2 = (lumaEnd2 < 0.0) != isLumaCenterSmaller;
	
	// Only keep the result in the direction of the closer side of the edge
	bool correctVariation = isDirection1 ? correctVariation1 : correctVariation2;
	
	// UV offset: read in the direction of the closest side of the edge
	float pixelOffset = - distanceFinal / edgeThickness + 0.5;
	
	// If the luma variation is incorrect, do not offset
	float finalOffset = correctVariation ? pixelOffset : 0.0;
	
	// Sub-pixel shifting for thin aliased geometry
	// Full weighted average of the luma over the 3x3 neighborhood
	// Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood
	float lumaAverage         = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
	float subPixelOffset1     = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
	float subPixelOffset2     = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;
	
	// Pick the biggest of the two offsets
	finalOffset = max(finalOffset, subPixelOffsetFinal);
	
	// Compute the final UV coordinates
	float2 finalUv = input.texCoord;
	if (isHorizontal) finalUv.y += finalOffset * stepLength;
    else              finalUv.x += finalOffset * stepLength;
		
	// Read the color at the new UV coordinates, and use it
	return Texture[0].Sample(SampleType[0], finalUv).rgb;
}

float4 Main(PixelInputType input) : SV_TARGET
{          	
	float4 baseColor = Texture[0].Load(int3(input.position.xy,0));
	
	if (u_applyFXAA == 1) baseColor.rgb = GetFXAAColor(input, baseColor);
	
	return baseColor;
}






























