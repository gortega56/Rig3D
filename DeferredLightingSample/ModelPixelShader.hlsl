struct Pixel
{
	float4 mPositionH	: SV_POSITION;
	float3 mTangent		: TANGENT;
	float3 mBitangent	: BINORMAL;
	float3 mNormal		: NORMAL;
	float2 mUV			: TEXCOORD;
};

struct PS_OUT
{
	float4  depth	: SV_TARGET0;
	float4	color	: SV_TARGET1;
	float4	normal	: SV_TARGET2;
};

cbuffer Color : register(b0)
{
	float4 color;
}

PS_OUT main(Pixel pixel) : SV_TARGET
{
	PS_OUT output;
	output.depth	= float4(pixel.mPositionH.z / pixel.mPositionH.w, 0.0f, 0.0f, 0.0);
	output.color	= color;
	output.normal	= (2.0f * normalize(float4(pixel.mNormal, 0.0f))) - 1.0f;
	return output;
}