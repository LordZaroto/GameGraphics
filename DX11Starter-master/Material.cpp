#include "Material.h"

Material::Material(DirectX::XMFLOAT4 colorTint, float roughness, DirectX::XMFLOAT2 scale, DirectX::XMFLOAT2 offset, std::shared_ptr<SimplePixelShader> pixelShader, std::shared_ptr<SimpleVertexShader> vertexShader)
{
	this->colorTint = colorTint;
	this->roughness = roughness;
	this->scale = scale;
	this->offset = offset;
	this->pixelShader = pixelShader;
	this->vertexShader = vertexShader;
}

Material::~Material()
{
}

void Material::SetColorTint(DirectX::XMFLOAT4 colorTint)
{
	this->colorTint = colorTint;
}

void Material::SetRoughness(float roughness)
{
	this->roughness = roughness;
}

void Material::SetScale(DirectX::XMFLOAT2 scale)
{
	this->scale = scale;
}

void Material::SetOffset(DirectX::XMFLOAT2 offset)
{
	this->offset = offset;
}

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader)
{
	this->pixelShader = pixelShader;
}

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader)
{
	this->vertexShader = vertexShader;
}

void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV)
{
	textureSRVs.insert({ name, textureSRV });
}

void Material::AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ name, sampler });
}

DirectX::XMFLOAT4 Material::GetColorTint()
{
	return colorTint;
}

float Material::GetRoughness()
{
	return roughness;
}

DirectX::XMFLOAT2 Material::GetScale()
{
	return scale;
}

DirectX::XMFLOAT2 Material::GetOffset()
{
	return offset;
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return pixelShader;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vertexShader;
}

void Material::PrepareMaterial()
{
	for (auto& t : textureSRVs) { pixelShader->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& t : samplers) { pixelShader->SetSamplerState(t.first.c_str(), t.second); }
}
