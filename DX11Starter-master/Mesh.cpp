#include "Mesh.h"

Mesh::Mesh(Vertex* vertecies, int vertexCount, unsigned int* indicies, int indexCount, 
	Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext)
{

}

Mesh::~Mesh()
{
}

ID3D11Buffer Mesh::GetVertexBuffer()
{
	return ID3D11Buffer();
}

ID3D11Buffer Mesh::GetIndexBuffer()
{
	return ID3D11Buffer();
}

int Mesh::GetIndexCount()
{
	return 0;
}

void Mesh::Draw()
{
}
