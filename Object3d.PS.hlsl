#include "Object3d.hlsli"

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
    float shininess;

};

struct DirectionalLight
{

    float4 color;
    float3 direction;
    float intensity;

};

struct Camera
{
	
    float3 worldPosition;
	
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

PixelShaderOutput main(VertexShaderOutput input)
{

    PixelShaderOutput output;

    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);

    output.color = gMaterial.color * textureColor;

    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float3 reflectLight = reflect(normalize(gDirectionalLight.direction), normalize(input.normal));

	
    if (gMaterial.enableLighting != 0)
    {

        float NdotL = dot(normalize(input.normal), normalize(-gDirectionalLight.direction));
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);

        float RdotE = dot(reflectLight, toEye);
        float specualarPow = pow(saturate(RdotE), gMaterial.shininess);
		
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
		
	//拡散反射
        float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
	
	//鏡面反射
        float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specualarPow * float3(1.0f, 1.0f, 1.0f);
	
	//拡散反射+鏡面反射
        output.color.rgb = diffuse + specular;
	
	//アルファ
        output.color.a = gMaterial.color.a * textureColor.a;

    }
    else
    {

        output.color = gMaterial.color * textureColor;

    }

    return output;

}