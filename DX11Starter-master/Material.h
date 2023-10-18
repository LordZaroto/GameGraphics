#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include "SimpleShader.h"
#include <memory>

class Material
{
public:
	Material(DirectX::XMFLOAT4 colorTint, float roughness, std::shared_ptr<SimplePixelShader> pixelShader,
		std::shared_ptr<SimpleVertexShader> vertexShader);
	~Material();

	//Setters
	void SetColorTint(DirectX::XMFLOAT4 colorTint);
	void SetRoughness(float roughness);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader);

	//Getters
	DirectX::XMFLOAT4 GetColorTint();
	float GetRoughness();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();

private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	float roughness;
};

