struct Pixel
{
	float4 mPositionH	: SV_POSITION;
	float3 mUV			: TEXCOORD;
};

Texture2D texture2d			: register(t0);
SamplerState samplerState	: register(s0);

float4 main(Pixel pixel) : SV_TARGET
{
	return texture2d.Sample(samplerState, pixel.mUV.xy);
}