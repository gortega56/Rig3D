struct Pixel
{
	float4 mPositionH	: SV_POSITION;
	float3 mUV			: TEXCOORD;
};

cbuffer blurParameters : register(b0)
{
	float2 uvOffsets[4];	// 4 byte float * 2 * 4 = 32
}

Texture2D sceneTexture		: register(t0);
SamplerState sceneSampler	: register(s0);

float4 main(Pixel pixel) : SV_TARGET
{
	//return sceneTexture.Sample(sceneSampler, pixel.mColor.xy);

	float4 pixelColor = float4(0, 0, 0, 0);
	pixelColor += sceneTexture.Sample(sceneSampler, pixel.mUV.xy + uvOffsets[0]);
	pixelColor += sceneTexture.Sample(sceneSampler, pixel.mUV.xy + uvOffsets[1]);
	pixelColor += sceneTexture.Sample(sceneSampler, pixel.mUV.xy - uvOffsets[1]);

	pixelColor /= 3;

	return pixelColor;

}