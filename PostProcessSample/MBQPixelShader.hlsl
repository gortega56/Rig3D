// Motion Blur Quad PS
// Resources: http://http.developer.nvidia.com/GPUGems3/gpugems3_ch27.html

struct Pixel
{
	float4 mPositionH	: SV_POSITION;
	float2 mUV			: TEXCOORD;
};

cbuffer transform : register(b0)
{
	matrix inverseClipMatrix;
	matrix previousClipMatrix;
}

Texture2D sceneTexture		: register(t0);
Texture2D depthTexture		: register(t1);
SamplerState sceneSampler	: register(s0);

float4 main(Pixel pixel) : SV_TARGET
{
	float zOverW = depthTexture.Sample(sceneSampler, pixel.mUV).r;

	// Viewport pos
	float4 H = float4(pixel.mUV.x * 2 - 1, (1 - pixel.mUV.y) * 2 - 1, zOverW, 1.0f);

	// Transform to world space
	float4 D = mul(H, inverseClipMatrix);
	
	// World Position
	float4 W = D / D.w;

	// Previous World Position
	float4 P = mul(W, previousClipMatrix);
	P /= P.w;

	// Velocity of pixel
	float2 V = ((W - P) * 0.5f).xy;

	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float2 uv = pixel.mUV;
	for (int i = 0; i < 2; i++, uv += V) {
		color += sceneTexture.Sample(sceneSampler, uv);
	}

	color /= 2;

	return color;
}