#pragma once

#include "DXCore.h"
#include "Vertex.h"
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>

class Mesh
{
public:
	Mesh(std::vector<Vertex> verts, int vertexCount, std::vector<UINT> indices, int indexCount,
		Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext);
	Mesh(const wchar_t* file, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext);
	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetIndexCount();
	void Draw();
	void InitMesh(std::vector<Vertex> verts, int vertexCount, std::vector<UINT> indices, int indexCount,
		Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	int indexCount;
};

