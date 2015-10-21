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
	if (zOverW == 1.0f) {
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
		float4 v = H - P;

		float2 V = ((v)* 0.5f).xy * float2(1.0f, -1.0f);

		float4 color = sceneTexture.Sample(sceneSampler, pixel.mUV);
		float2 uv = pixel.mUV + V;
		float n = 50;
		for (int i = 0; i < n; i++, uv += V) {
			color += sceneTexture.Sample(sceneSampler, uv);
		}

		color /= n;

		return color;
	}

	return  sceneTexture.Sample(sceneSampler, pixel.mUV);
}