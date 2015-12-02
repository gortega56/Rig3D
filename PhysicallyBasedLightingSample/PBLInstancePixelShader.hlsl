struct Pixel
{
	float4 position		: SV_POSITION;
	float3 positionT	: POSITIONT;
	float3 normal		: NORMAL;
};

TextureCube textureCube : register(t0);

cbuffer BRDF_Inputs : register(b0)
{
	float3 L;
	float3 V;
}

float4 D(float h)
{

}

float4 F(float d)
{

}

float4 G(float l, float v)
{

}

float4 main(Pixel pixel) : SV_TARGET
{
	float3 N = normalize(pixel.normal);
	float3 H = normalize((L + V) * 0.5f);
	float cosL = dot(N, L);
	float cosV = dot(N, V);
	float cosD = dot(H, L);
	float cosH = dot(N, H);
	float l = acos(cosL);
	float v = acos(cosV);
	float d = acos(cosD);
	float h = acos(cosH);
	float4 diffuse = saturate(cosL);

	return diffuse + ((D(h) * F(d) * G(l, v)) / (4.0f * cosL * cosV);

	// Direct Diffuse = N dot L / PI

	// Have to calculate Fresnel - F0 to use with DS and IDS.

	// Direct Specular = Cook Torrance Microfacet 
		// - Roughness
		// - Metalness
	
	// Indirect Diffuse = Spherical Harmonics  = Another Cubemap
		// 


	// Indirect Specular = Create cubemap
		// - Environment BRDF
		// - Roughness
		// - Metalness
}