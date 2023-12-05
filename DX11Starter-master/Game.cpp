#include "Game.h"
#include "Mesh.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "BufferStructs.h"
#include <memory>
#include <vector>

#include "WICTextureLoader.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

//ImGui constant buffer
//Throws errors if in the header file for some reason
XMFLOAT4 IMGUI_colorTint;
XMFLOAT4X4 IMGUI_world;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

	//Cameras
	std::shared_ptr<Camera> camera = std::make_shared<Camera>(	(float)this->windowWidth / this->windowHeight, 
		DirectX::XMFLOAT3(0.0f, 0.0f, -4.0f), 45.0f, 0.01f, 1000.0f, 3.0f, 0.01f, false);

	std::shared_ptr<Camera> camera1 = std::make_shared<Camera>((float)this->windowWidth / this->windowHeight,
		DirectX::XMFLOAT3(0.0f, 2.0f, -3.0f), 108.0f, 0.1f, 100.0f, 3.0f, 0.01f, false);

	std::shared_ptr<Camera> camera2 = std::make_shared<Camera>((float)this->windowWidth / this->windowHeight,
		DirectX::XMFLOAT3(1.0f, -2.0f, -5.0f), 140.0f, 0.1f, 150.0f, 3.0f, 0.01f, false);

	activeCameraIndex = 0;

	cameras = { camera, camera1, camera2 };

	shadowMapRes = 1024.0f;
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs

	//ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();
	
	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
	}

	//Get the constant buffer size
	unsigned int size = sizeof(VertexShaderExternalData);
	size = (size + 15) / 16 * 16;

	//Describe the constant buffer
	D3D11_BUFFER_DESC cbDesc	= {};
	cbDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth			= size;
	cbDesc.CPUAccessFlags		= D3D10_CPU_ACCESS_WRITE;
	cbDesc.Usage				= D3D11_USAGE_DYNAMIC;

	//Initialize Colortint and offset shader
	IMGUI_colorTint = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	IMGUI_world = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	//Lights
	ambientColor = XMFLOAT3(0.969f, 0.6f, 0.0f);

	directionalLight = {};
	directionalLight.Type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight.Direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
	directionalLight.Color = XMFLOAT3(1.0f, 0.0f, 0.0f);
	directionalLight.Intensity = 5.0f;

	directionalLight2 = {};
	directionalLight2.Type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	directionalLight2.Color = XMFLOAT3(0.0f, 1.0f, 0.0f);
	directionalLight2.Intensity = 5.0f;

	directionalLight3 = {};
	directionalLight3.Type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight3.Direction = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	directionalLight3.Color = XMFLOAT3(0.0f, 0.0f, 1.0f);
	directionalLight3.Intensity = 5.0f;
	
	pointLight = {};
	pointLight.Type = LIGHT_TYPE_POINT;
	pointLight.Position = XMFLOAT3(-3.0f, 2.0f, -2.0f);
	pointLight.Color = XMFLOAT3(0.0f, 0.5f, 0.8f);
	pointLight.Intensity = 5.0f;
	pointLight.Range = 10.0f;

	pointLight2 = {};
	pointLight2.Type = LIGHT_TYPE_POINT;
	pointLight2.Position = XMFLOAT3(3.0f, -2.0f, -2.0f);
	pointLight2.Color = XMFLOAT3(0.8f, 0.5f, 0.0f);
	pointLight2.Intensity = 5.0f;
	pointLight.Range = 10.0f;

	//Shadow Map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapRes;
	shadowDesc.Height = shadowMapRes;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture.Get(), &shadowDSDesc, shadowDSV.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture.Get(), &srvDesc, shadowSRV.GetAddressOf());

	//Creating light matricies
	XMVECTOR direction = XMVectorSet(directionalLight.Direction.x, directionalLight.Direction.y, directionalLight.Direction.z, 0.0f);
	XMMATRIX lightView = XMMatrixLookToLH(-direction * 20, direction, XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&lightViewMatrix, lightView);

	float lightProjectionSize = 15.0f;
	XMMATRIX lightProjection = XMMatrixOrthographicLH(lightProjectionSize, lightProjectionSize, 1.0f, 100.0f);
	XMStoreFloat4x4(&lightProjectMatrix, lightProjection);


	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());

	ImGui::StyleColorsDark(); //Light, Classic, Dark
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader.cso").c_str());
	psCustom = std::make_shared<SimplePixelShader>(device, context, FixPath(L"CustomPS.cso").c_str());
	skyPixelShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"SkyPixelShader.cso").c_str());
	skyVertexShader = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"SkyVertexShader.cso").c_str());
	shadowVS = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"ShadowVertexShader.cso").c_str());
}



// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	//Textures
	//---------------------------------------
	//bronze
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/bronze_albedo.png").c_str(), 0, bronzeAlbedo.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/bronze_normals.png").c_str(), 0, bronzeNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/bronze_roughness.png").c_str(), 0, bronzeRoughness.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/bronze_metal.png").c_str(), 0, bronzeMetal.GetAddressOf());

	//cobblestone
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/cobblestone_albedo.png").c_str(), 0, cobblestoneAlbedo.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/cobblestone_normals.png").c_str(), 0, cobblestoneNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/cobblestone_roughness.png").c_str(), 0, cobblestoneRoughness.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/cobblestone_metal.png").c_str(), 0, cobblestoneMetal.GetAddressOf());

	//floor
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/floor_albedo.png").c_str(), 0, floorAlbedo.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/floor_normals.png").c_str(), 0, floorNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/floor_roughness.png").c_str(), 0, floorRoughness.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/floor_metal.png").c_str(), 0, floorMetal.GetAddressOf());

	//paint
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/paint_albedo.png").c_str(), 0, paintAlbedo.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/paint_normals.png").c_str(), 0, paintNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/paint_roughness.png").c_str(), 0, paintRoughness.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/paint_metal.png").c_str(), 0, paintMetal.GetAddressOf());

	//scratched
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/scratched_albedo.png").c_str(), 0, scratchedAlbedo.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/scratched_normals.png").c_str(), 0, scratchedNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/scratched_roughness.png").c_str(), 0, scratchedRoughness.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/PBR/scratched_metal.png").c_str(), 0, scratchedMetal.GetAddressOf());


	//Samplers
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, sampler.GetAddressOf());

	//Skybox
	sky = Sky(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context),
		sampler,
		device,
		context,
		skyVertexShader,
		skyPixelShader,
		FixPath(L"../../Assets/Skies/Planet/right.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/left.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/up.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/down.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/front.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/back.png").c_str());
	
	//Create Materials
	material = std::make_shared<Material>(
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		0.99f,
		XMFLOAT2(2.0f, 2.0f),
		XMFLOAT2(0.0f, 0.0f),
		pixelShader,
		vertexShader);
	material1 = std::make_shared<Material>(
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		0.99f,
		XMFLOAT2(2.0f, 2.0f),
		XMFLOAT2(0.0f, 0.0f),
		pixelShader,
		vertexShader);
	material2 = std::make_shared<Material>(
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		0.99f,
		XMFLOAT2(2.0f, 2.0f),
		XMFLOAT2(0.0f, 0.0f),
		pixelShader,
		vertexShader);
	material3 = std::make_shared<Material>(
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		0.99f,
		XMFLOAT2(2.0f, 2.0f),
		XMFLOAT2(0.0f, 0.0f),
		pixelShader,
		vertexShader);
	material4 = std::make_shared<Material>(
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		0.99f,
		XMFLOAT2(2.0f, 2.0f),
		XMFLOAT2(0.0f, 0.0f),
		pixelShader,
		vertexShader);

	//Add Textures and Samplers to Materials
	//Samplers
	material->AddSampler("BasicSampler", sampler);
	material1->AddSampler("BasicSampler", sampler);
	material2->AddSampler("BasicSampler", sampler);
	material3->AddSampler("BasicSampler", sampler);
	material4->AddSampler("BasicSampler", sampler);
	//bronze
	material->AddTextureSRV("Albedo", bronzeAlbedo);
	material->AddTextureSRV("NormalMap", bronzeNormal);
	material->AddTextureSRV("RoughnessMap", bronzeRoughness);
	material->AddTextureSRV("MetalnessMap", bronzeMetal);
	//cobblestone
	material1->AddTextureSRV("Albedo", cobblestoneAlbedo);
	material1->AddTextureSRV("NormalMap", cobblestoneNormal);
	material1->AddTextureSRV("RoughnessMap", cobblestoneRoughness);
	material1->AddTextureSRV("MetalnessMap", cobblestoneMetal);
	//floor
	material2->AddTextureSRV("Albedo", floorAlbedo);
	material2->AddTextureSRV("NormalMap", floorNormal);
	material2->AddTextureSRV("RoughnessMap", floorRoughness);
	material2->AddTextureSRV("MetalnessMap", floorMetal);
	//paint
	material3->AddTextureSRV("Albedo", paintAlbedo);
	material3->AddTextureSRV("NormalMap", paintNormal);
	material3->AddTextureSRV("RoughnessMap", paintRoughness);
	material3->AddTextureSRV("MetalnessMap", paintMetal);
	//scratched
	material4->AddTextureSRV("Albedo", scratchedAlbedo);
	material4->AddTextureSRV("NormalMap", scratchedNormal);
	material4->AddTextureSRV("RoughnessMap", scratchedRoughness);
	material4->AddTextureSRV("MetalnessMap", scratchedMetal);

	std::shared_ptr<Entity> entity = std::make_shared<Entity>(
		std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context), material);
	std::shared_ptr<Entity> entity1 = std::make_shared<Entity>(
		std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str(), device, context), material1);
	std::shared_ptr<Entity> entity2 = std::make_shared<Entity>(
		std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), device, context), material2);
	std::shared_ptr<Entity> entity3 = std::make_shared<Entity>(
		std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device, context), material3);
	std::shared_ptr<Entity> entity4 = std::make_shared<Entity>(
		std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context), material4);

	//Move entities
	entity->GetTransform()->SetPosition(XMFLOAT3(-5.0, 0.0, 0.0));
	entity1->GetTransform()->SetPosition(XMFLOAT3(-2.5, 0.0, 0.0));
	entity2->GetTransform()->SetPosition(XMFLOAT3(0.0, 0.0, 0.0));
	entity3->GetTransform()->SetPosition(XMFLOAT3(2.5, 0.0, 0.0));
	entity4->GetTransform()->SetPosition(XMFLOAT3(0.0, -12.0, 0.0));

	//Scale entities
	entity4->GetTransform()->SetScale(XMFLOAT3(10.0, 10.0, 10.0));
	
	//Add all entities to the entity vector
	entities.push_back(entity);
	entities.push_back(entity1);
	entities.push_back(entity2);
	entities.push_back(entity3);
	entities.push_back(entity4);
}


// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	for (int i = 0; i < cameras.size(); i++)
	{
		cameras[i]->UpdateProjectionMatrix((float)this->windowWidth / this->windowHeight);
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	ImGuiUpdate(deltaTime, totalTime);

	//Move entities
	//entities[0]->GetTransform()->Scale((deltaTime/10)+1, (deltaTime / 10) + 1, 1);
	//entities[1]->GetTransform()->MoveAbsolute(deltaTime/100, deltaTime/100, 0);
	//entities[2]->GetTransform()->Rotate(0, 0, deltaTime/100);

	//Camera
	cameras[activeCameraIndex]->Update(deltaTime);
	
	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		//Clear Shadow Map
		context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		//Setup output merger state
		ID3D11RenderTargetView* nullRTV{};
		context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

		//Deactivate Pixel Shader
		context->PSSetShader(0, 0, 0);

		//Change viewport
		D3D11_VIEWPORT viewport = {};
		viewport.Width = shadowMapRes;
		viewport.Height = shadowMapRes;
		viewport.MaxDepth = 1.0f;
		context->RSSetViewports(1, &viewport);

		shadowVS->SetShader();
		shadowVS->SetMatrix4x4("view", lightViewMatrix);
		shadowVS->SetMatrix4x4("projection", lightProjectMatrix);

		//Drawing each entity
		for (int i = 0; i < entities.size(); i++)
		{
			shadowVS->SetMatrix4x4("world", entities[i]->GetTransform()->GetWorldMatrix());
			shadowVS->CopyAllBufferData();

			entities[i]->GetMesh()->Draw();
		}

		//Reset Pipeline
		viewport.Width = (float)this->windowWidth;
		viewport.Height = (float)this->windowHeight;
		context->RSSetViewports(1, &viewport);
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}

	//Drawing each entity
	for (int i = 0; i < entities.size(); i++)
	{
		std::shared_ptr<Material> mat = entities[i]->GetMaterial();
		
		std::shared_ptr<SimpleVertexShader> vs = mat->GetVertexShader();
		vs->SetMatrix4x4("world", entities[i]->GetTransform()->GetWorldMatrix());
		vs->SetMatrix4x4("worldInverseTranspose", entities[i]->GetTransform()->GetWorldInverseTransposeMatrix());
		vs->SetMatrix4x4("view", cameras[activeCameraIndex]->GetViewMatrix());
		vs->SetMatrix4x4("projection", cameras[activeCameraIndex]->GetProjectionMatrix());
		vs->SetMatrix4x4("lightView", lightViewMatrix);
		vs->SetMatrix4x4("lightProjection", lightProjectMatrix);

		vs->CopyAllBufferData();

		std::shared_ptr<SimplePixelShader> ps = mat->GetPixelShader();
		ps->SetFloat4("colorTint", mat->GetColorTint());
		ps->SetFloat3("cameraPos", cameras[activeCameraIndex]->GetTransform()->GetPosition());
		ps->SetFloat("roughness", mat->GetRoughness());
		ps->SetFloat3("ambient", ambientColor);
		ps->SetFloat2("scale", mat->GetScale());
		ps->SetFloat2("offset", mat->GetOffset());
		ps->SetData("directionalLight", &directionalLight, sizeof(Light));
		ps->SetData("directionalLight2", &directionalLight2, sizeof(Light));
		ps->SetData("directionalLight3", &directionalLight3, sizeof(Light));
		ps->SetData("pointLight", &pointLight, sizeof(Light));
		ps->SetData("pointLight2", &pointLight2, sizeof(Light));

		mat->PrepareMaterial();

		ps->CopyAllBufferData();
		
		vs->SetShader();
		ps->SetShader();
		entities[i]->GetMesh()->Draw();
	}

	//Drawing the sky
	sky.Draw(cameras[activeCameraIndex]);

	//ImGui
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;
		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}

// Stuff done every frame for ImGui
void Game::ImGuiUpdate(float deltaTime, float totalTime)
{
	// Feed fresh input data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input& input = Input::GetInstance();
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);

	// Show the demo window
	//ImGui::ShowDemoWindow();

	//Making a ImGui window
	ImGui::Begin("My First Window");

	if (ImGui::CollapsingHeader("Stats"))
	{
		ImGui::Text("Framerate: (%g)", ImGui::GetIO().Framerate);
		ImGui::Text("Height: (%g)", windowHeight);
		ImGui::Text("Width: (%g)", windowWidth);
	}

	if (ImGui::CollapsingHeader("Camera"))
	{

		if (ImGui::TreeNode("Camera List"))
		{
			const char* cameraNames[] = { "Camera 1", "Camera 2", "Camera 3"};
			static int currentCamera = 0;

			if (ImGui::BeginListBox("Cameras"))
			{
				for (int i = 0; i < IM_ARRAYSIZE(cameraNames); i++)
				{
					const bool isSelected = (currentCamera == i);
					if (ImGui::Selectable(cameraNames[i], isSelected))
						currentCamera = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();

					activeCameraIndex = currentCamera;
				}
				ImGui::EndListBox();

			}

			ImGui::TreePop();
		}

		XMFLOAT3 camPosVec = cameras[activeCameraIndex]->GetTransform()->GetPosition();

		if (ImGui::TreeNode("Camera Stats"))
		{
			XMFLOAT3 camPosVec = cameras[activeCameraIndex]->GetTransform()->GetPosition();
			//position
			ImGui::Text("Position: (%g, %g, %g)", camPosVec.x, camPosVec.y, camPosVec.z);
			//fov
			ImGui::Text("Field of View: (%g)", cameras[activeCameraIndex]->GetFOV());
			//near plane
			ImGui::Text("Near Plane: (%g)", cameras[activeCameraIndex]->GetNearPlane());
			//far plane
			ImGui::Text("Far Plane: (%g)", cameras[activeCameraIndex]->GetFarPlane());
			//orthographic
			ImGui::Text("Orthographic: (%g)", cameras[activeCameraIndex]->GetIsOrthographic());
			ImGui::TreePop();
		}
	}

	//Variables to interact with ImGui
	// I hate this, I tried to make it nice with loops and vectors
	// Does not work. ImGui only wants one form. anger
	//Entity 0
	XMFLOAT3 posVec = entities[0]->GetTransform()->GetPosition();
	static float pos[3] = { posVec.x, posVec.y, posVec.z };
	XMFLOAT3 rotVec = entities[0]->GetTransform()->GetPitchYawRoll();
	static float rot[3] = { rotVec.x, rotVec.y, rotVec.z};
	XMFLOAT3 sclVec = entities[0]->GetTransform()->GetScale();
	static float scl[3] = { sclVec.x, sclVec.y, sclVec.z};

	//Entity 1
	XMFLOAT3 posVec1 = entities[1]->GetTransform()->GetPosition();
	static float pos1[3] = { posVec1.x, posVec1.y, posVec1.z };
	XMFLOAT3 rotVec1 = entities[1]->GetTransform()->GetPitchYawRoll();
	static float rot1[3] = { rotVec1.x, rotVec1.y, rotVec1.z };
	XMFLOAT3 sclVec1 = entities[1]->GetTransform()->GetScale();
	static float scl1[3] = { sclVec1.x, sclVec1.y, sclVec1.z };

	//Entity 2
	XMFLOAT3 posVec2 = entities[2]->GetTransform()->GetPosition();
	static float pos2[3] = { posVec2.x, posVec2.y, posVec2.z };
	XMFLOAT3 rotVec2 = entities[2]->GetTransform()->GetPitchYawRoll();
	static float rot2[3] = { rotVec2.x, rotVec2.y, rotVec2.z };
	XMFLOAT3 sclVec2 = entities[2]->GetTransform()->GetScale();
	static float scl2[3] = { sclVec2.x, sclVec2.y, sclVec2.z };

	//Entity 3
	XMFLOAT3 posVec3 = entities[3]->GetTransform()->GetPosition();
	static float pos3[3] = { posVec3.x, posVec3.y, posVec3.z };
	XMFLOAT3 rotVec3 = entities[3]->GetTransform()->GetPitchYawRoll();
	static float rot3[3] = { rotVec3.x, rotVec3.y, rotVec3.z };
	XMFLOAT3 sclVec3 = entities[3]->GetTransform()->GetScale();
	static float scl3[3] = { sclVec3.x, sclVec3.y, sclVec3.z };

	//Entity 4
	XMFLOAT3 posVec4 = entities[4]->GetTransform()->GetPosition();
	static float pos4[3] = { posVec4.x, posVec4.y, posVec4.z };
	XMFLOAT3 rotVec4 = entities[4]->GetTransform()->GetPitchYawRoll();
	static float rot4[3] = { rotVec4.x, rotVec4.y, rotVec4.z };
	XMFLOAT3 sclVec4 = entities[4]->GetTransform()->GetScale();
	static float scl4[3] = { sclVec4.x, sclVec4.y, sclVec4.z };

	//All meshes
	static float vec4f[4] = { IMGUI_colorTint.x, IMGUI_colorTint.y, IMGUI_colorTint.z, IMGUI_colorTint.w };
	
	if (ImGui::CollapsingHeader("All Meshes"))
	{
		//ImGui::DragFloat3("Offset", vec3f, 0.01f, -1.0f, 1.0f);
		ImGui::ColorEdit4("Color Tint", vec4f);
	}

	if (ImGui::CollapsingHeader("Entities"))
	{
		if (ImGui::TreeNode("Entity 0"))
		{
			ImGui::DragFloat3("Position", pos, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat3("Rotation", rot, 0.01f, -XM_2PI, XM_2PI);
			ImGui::DragFloat3("Scale", scl, 0.01f, 1.0f, 5.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Entity 1"))
		{
			ImGui::DragFloat3("Position", pos1, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat3("Rotation", rot1, 0.01f, -XM_2PI, XM_2PI);
			ImGui::DragFloat3("Scale", scl1, 0.01f, 1.0f, 5.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Entity 2"))
		{
			ImGui::DragFloat3("Position", pos2, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat3("Rotation", rot2, 0.01f, -XM_2PI, XM_2PI);
			ImGui::DragFloat3("Scale", scl2, 0.01f, 1.0f, 5.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Entity 3"))
		{
			ImGui::DragFloat3("Position", pos3, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat3("Rotation", rot3, 0.01f, -XM_2PI, XM_2PI);
			ImGui::DragFloat3("Scale", scl3, 0.01f, 1.0f, 5.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Entity 4"))
		{
			ImGui::DragFloat3("Position", pos4, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat3("Rotation", rot4, 0.01f, -XM_2PI, XM_2PI);
			ImGui::DragFloat3("Scale", scl4, 0.01f, 1.0f, 5.0f);
			ImGui::TreePop();
		}
	}

	//Light colors
	static float light[3] = { directionalLight.Color.x, directionalLight.Color.y, directionalLight.Color.z};
	static float light2[3] = { directionalLight2.Color.x, directionalLight2.Color.y, directionalLight2.Color.z };
	static float light3[3] = { directionalLight3.Color.x, directionalLight3.Color.y, directionalLight3.Color.z };
	static float light4[3] = { pointLight.Color.x, pointLight.Color.y, pointLight.Color.z };
	static float light5[3] = { pointLight2.Color.x, pointLight2.Color.y, pointLight2.Color.z };

	if (ImGui::CollapsingHeader("Lights"))
	{
		if (ImGui::TreeNode("Light 0"))
		{
			ImGui::ColorEdit3("Color", light);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Light 1"))
		{
			ImGui::ColorEdit3("Color", light2);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Light 2"))
		{
			ImGui::ColorEdit3("Color", light3);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Light 3"))
		{
			ImGui::ColorEdit3("Color", light4);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Light 4"))
		{
			ImGui::ColorEdit3("Color", light5);
			ImGui::TreePop();
		}
	}

	//Set the ImGui changes
	entities[0]->GetTransform()->SetPosition((XMFLOAT3)pos);
	entities[0]->GetTransform()->SetRotation((XMFLOAT3)rot);
	entities[0]->GetTransform()->SetScale((XMFLOAT3)scl);

	entities[1]->GetTransform()->SetPosition((XMFLOAT3)pos1);
	entities[1]->GetTransform()->SetRotation((XMFLOAT3)rot1);
	entities[1]->GetTransform()->SetScale((XMFLOAT3)scl1);

	entities[2]->GetTransform()->SetPosition((XMFLOAT3)pos2);
	entities[2]->GetTransform()->SetRotation((XMFLOAT3)rot2);
	entities[2]->GetTransform()->SetScale((XMFLOAT3)scl2);

	entities[3]->GetTransform()->SetPosition((XMFLOAT3)pos3);
	entities[3]->GetTransform()->SetRotation((XMFLOAT3)rot3);
	entities[3]->GetTransform()->SetScale((XMFLOAT3)scl3);

	entities[4]->GetTransform()->SetPosition((XMFLOAT3)pos4);
	entities[4]->GetTransform()->SetRotation((XMFLOAT3)rot4);
	entities[4]->GetTransform()->SetScale((XMFLOAT3)scl4);

	IMGUI_colorTint.x = vec4f[0];
	IMGUI_colorTint.y = vec4f[1];
	IMGUI_colorTint.z = vec4f[2];
	IMGUI_colorTint.w = vec4f[3];

	//Light colors
	directionalLight.Color.x = light[0];
	directionalLight.Color.y = light[1];
	directionalLight.Color.z = light[2];

	directionalLight2.Color.x = light2[0];
	directionalLight2.Color.y = light2[1];
	directionalLight2.Color.z = light2[2];

	directionalLight3.Color.x = light3[0];
	directionalLight3.Color.y = light3[1];
	directionalLight3.Color.z = light3[2];

	pointLight.Color.x = light4[0];
	pointLight.Color.y = light4[1];
	pointLight.Color.z = light4[2];

	pointLight2.Color.x = light5[0];
	pointLight2.Color.y = light5[1];
	pointLight2.Color.z = light5[2];

	ImGui::End();
}
