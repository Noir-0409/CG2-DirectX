#include "Object3d.hlsli"

struct PixelShaderOutput {

	float4 color : SV_TARGET0;

};

struct Material {

	float4 color;

};

ConstantBuffer<Material> gMaterial:register(b0);

PixelShaderOutput main(VertexShaderOutput input) {

	PixelShaderOutput output;

	output.color = gMaterial.color;

	return output;

}