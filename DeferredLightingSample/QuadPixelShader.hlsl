struct Pixel
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD;
};

cbuffer ambientLight	: register(b0)
{
	float4	ambientLightColor;
}

Texture2D diffuseMap	: register(t0);
Texture2D lightMap		: register(t1);

SamplerState samplerState : register(s0);

float4 main(Pixel pixel) : SV_TARGET
{
	return ambientLightColor + diffuseMap.Sample(samplerState, pixel.uv) * lightMap.Sample(samplerState, pixel.uv);
}