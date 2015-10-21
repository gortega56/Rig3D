struct Pixel
{
	float4 mPositionH	: SV_POSITION;
	float2 mUV			: TEXCOORD;
};

Texture2D gBufferMap		: register(t0);

SamplerState samplerState	: register(s0);

float4 main(Pixel pixel) : SV_TARGET
{
	return gBufferMap.Sample(samplerState, pixel.mUV);
}