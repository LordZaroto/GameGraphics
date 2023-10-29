#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include "SimpleShader.h"
#include <memory>

class Material
{
public:
	Material(DirectX::XMFLOAT4 colorTint, float roughness, DirectX::XMFLOAT2 scale, DirectX::XMFLOAT2 offset,
		std::shared_ptr<SimplePixelShader> pixelShader, std::shared_ptr<SimpleVertexShader> vertexShader);
	~Material();

	//Setters
	void SetColorTint(DirectX::XMFLOAT4 colorTint);
	void SetRoughness(float roughness);
	void SetScale(DirectX::XMFLOAT2 scale);
	void SetOffset(DirectX::XMFLOAT2 offset);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader);
	void AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV);
	void AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	//Getters
	DirectX::XMFLOAT4 GetColorTint();
	float GetRoughness();
	DirectX::XMFLOAT2 GetScale();
	DirectX::XMFLOAT2 GetOffset();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();

	//Helpers
	void PrepareMaterial();

private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	float roughness;
	DirectX::XMFLOAT2 scale;
	DirectX::XMFLOAT2 offset;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};

