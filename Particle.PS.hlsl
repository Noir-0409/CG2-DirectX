#include "Particle.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{

	float4 color : SV_TARGET0;

};

struct Material
{

	float4 color;
	int enableLighting;
	float4x4 uvTransform;

};

struct DirectionalLight
{

	float4 color;
	float3 direction;
	float intensity;

};

ConstantBuffer<Material> gMaterial : register(b0);

//ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

PixelShaderOutput main(VertexShaderOutput input)
{

	PixelShaderOutput output;
	float4 transformdUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
	float4 textureColor = gTexture.Sample(gSampler, transformdUV.xy);
	output.color = gMaterial.color * textureColor;
	
	if (output.color.a == 0.0)
	{
		
		discard;
		
	}
	
	return output;

}