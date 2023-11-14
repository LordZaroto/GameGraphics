#pragma once

#include "DXCore.h"
#include "Mesh.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include <vector>
#include <memory>
#include "Entity.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"
#include "Lights.h"
#include "Sky.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	//Helper methods
	void ImGuiUpdate(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders(); 
	void CreateGeometry();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	//Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	//Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	
	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> psCustom;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> skyPixelShader;
	std::shared_ptr<SimpleVertexShader> skyVertexShader;

	//Camera
	std::vector<std::shared_ptr<Camera>> cameras;
	int activeCameraIndex;

	//List of meshes
	std::vector<std::shared_ptr<Entity>> entities;

	//List of Materials
	std::shared_ptr<Material> material;
	std::shared_ptr<Material> material1;
	std::shared_ptr<Material> material2;

	//Lights
	DirectX::XMFLOAT3 ambientColor;
	Light directionalLight;
	Light directionalLight2;
	Light directionalLight3;
	Light pointLight;
	Light pointLight2;

	//Textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cushionDiffuse;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cushionNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockDiffuse;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockNormal;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

	//SkyBox
	Sky sky;
};

