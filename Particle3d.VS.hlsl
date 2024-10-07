#include "Particle3d.hlsli"

struct TransformationMatrix
{

	float4x4 WVP;
	float4x4 World;

};

StructuredBuffer<TransformationMatrix> gTransformationMatrix : register(t0);

struct VertexShaderInput
{

	float4 position : POSITION0;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL0;

};

VertexShaderOutput main(VertexShaderInput input,uint32_t instanceId:SV_InstanceID)
{

	VertexShaderOutput output;

	output.position = mul(input.position, gTransformationMatrices[InstanceID].WVP);

	output.texcoord = input.texcoord;

	output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrices[InstanceID].world));

	return output;

}