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

Texture2D positionMap	: register(t0);
Texture2D depthMap		: register(t1);
Texture2D diffuseMap	: register(t2);
Texture2D normalMap		: register(t3);

SamplerState samplerState : register(s0);


float4 main(Pixel pixel) : SV_TARGET
{
	float4 lightColor = {1.0f, 1.0f, 1.0f, 1.0f};
	float3 lightDirection = normalize(float3(1.0f, -1.0f, 0.5f));

	float3 position = positionMap.Sample(samplerState, pixel.mUV).xyz;
	float depth = depthMap.Sample(samplerState, pixel.mUV).r;
	float4 color = diffuseMap.Sample(samplerState, pixel.mUV);
	float3 normal = normalize(normalMap.Sample(samplerState, pixel.mUV) * 0.5f - 1.0f).xyz;

	return color * saturate(dot(-lightDirection, normal));
}