#include "Game.h"
#include "Mesh.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "BufferStructs.h"
#include <memory>
#include <vector>

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

	std::shared_ptr<Camera> camera = std::make_shared<Camera>(	(float)this->windowWidth / this->windowHeight, 
		DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), 45, 0.01f, 1000, 6, 0.5f, false);

	std::shared_ptr<Camera> camera1 = std::make_shared<Camera>((float)this->windowWidth / this->windowHeight,
		DirectX::XMFLOAT3(0.0f, 2.0f, -3.0f), 108, 0.1f, 100, 6, 0.5f, false);

	std::shared_ptr<Camera> camera2 = std::make_shared<Camera>((float)this->windowWidth / this->windowHeight,
		DirectX::XMFLOAT3(1.0f, -2.0f, -5.0f), 140, 0.1f, 150, 6, 0.5f, false);

	activeCameraIndex = 0;

	cameras = { camera, camera1, camera2 };
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
		context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		context->VSSetShader(vertexShader.Get(), 0, 0);
		context->PSSetShader(pixelShader.Get(), 0, 0);
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

	device->CreateBuffer(&cbDesc, 0, vsConstantBuffer.GetAddressOf());

	//Initialize Colortint and offset shader
	IMGUI_colorTint = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	IMGUI_world = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

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
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}



// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself


	//Colors
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	//Mesh 1
	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.3f, +0.0f), red },
		{ XMFLOAT3(+0.3f, -0.3f, +0.0f), blue },
		{ XMFLOAT3(-0.3f, -0.3f, +0.0f), green },
	};

	unsigned int indices[] = { 0, 1, 2 };

	std::shared_ptr<Mesh> triangle = std::make_shared<Mesh>(vertices, (sizeof(vertices)/sizeof(vertices[0])), indices, 
		(sizeof(indices) / sizeof(indices[0])), device, context);

	//Mesh 2
	Vertex vertices2[] =
	{
		{ XMFLOAT3(+0.7f, +0.8f, +0.0f), black },
		{ XMFLOAT3(+0.9f, +0.5f, +0.0f), black },
		{ XMFLOAT3(+0.8f, +0.3f, +0.0f), red },
		{ XMFLOAT3(+0.6f, +0.3f, +0.0f), red },
		{ XMFLOAT3(+0.7f, +0.0f, +0.0f), black },
		{ XMFLOAT3(+0.9f, +0.2f, +0.0f), black },
	};

	unsigned int indices2[] = { 0, 1, 2, 0, 2, 3, 3, 2, 4, 2, 5, 4};

	std::shared_ptr<Mesh> triangle2 = std::make_shared<Mesh>(vertices2, (sizeof(vertices2) / sizeof(vertices2[0])), indices2,
		(sizeof(indices2) / sizeof(indices2[0])), device, context);

	//Mesh 3
	Vertex vertices3[] =
	{
		{ XMFLOAT3(-0.8f, +0.5f, +0.0f), black },
		{ XMFLOAT3(-0.4f, +0.5f, +0.0f), black },
		{ XMFLOAT3(-0.8f, +0.3f, +0.0f), white },
		{ XMFLOAT3(-0.4f, +0.3f, +0.0f), white },
	};

	unsigned int indices3[] = { 0, 1, 2, 1, 3, 2};

	std::shared_ptr<Mesh> triangle3 = std::make_shared<Mesh>(vertices3, (sizeof(vertices3) / sizeof(vertices3[0])), indices3,
		(sizeof(indices3) / sizeof(indices3[0])), device, context);

	std::shared_ptr<Entity> entity = std::make_shared<Entity>(triangle);
	std::shared_ptr<Entity> entity1 = std::make_shared<Entity>(triangle2);
	std::shared_ptr<Entity> entity2 = std::make_shared<Entity>(triangle2);
	std::shared_ptr<Entity> entity3 = std::make_shared<Entity>(triangle3);
	std::shared_ptr<Entity> entity4 = std::make_shared<Entity>(triangle3);
	
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
	//entities[2]->GetTransform()->Rotate(0, 0, deltaTime);

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
	}

	//Drawing each entity
	for (int i = 0; i < entities.size(); i++)
	{
		VertexShaderExternalData vsData;
		vsData.colorTint = IMGUI_colorTint;
		vsData.world = entities[i]->GetTransform()->GetWorldMatrix();
		vsData.view = cameras[activeCameraIndex]->GetViewMatrix();
		vsData.projection = cameras[activeCameraIndex]->GetProjectionMatrix();

		D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
		context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

		memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));

		context->Unmap(vsConstantBuffer.Get(), 0);

		context->VSSetConstantBuffers(
			0, //Which slot (register) to bind the buffer to?
			1, //How many are we activating?
			vsConstantBuffer.GetAddressOf()); //Array of buffers
		
		entities[i]->GetMesh()->Draw();
	}

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

	ImGui::End();
}
