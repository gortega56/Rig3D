struct Pixel
{
	float4 mPositionH	: SV_POSITION;
	float2 mUV			: TEXCOORD;
};

struct PointLight 
{
	float3 position;
};

cbuffer pointLights		: register(b0)
{
	PointLight pointLight;
}

Texture2D depthMap		: register(t0);
Texture2D diffuseMap	: register(t1);
Texture2D normalMap		: register(t2);

SamplerState samplerState : register(s0);


float4 main(Pixel pixel) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}