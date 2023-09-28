#include "Transform.h"

// For the DirectX Math library
using namespace DirectX;

Transform::Transform()
{
	position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	pitchYawRoll = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	forward = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
	isMatrixChanged = false;
	isRotationChanged = false;
}

Transform::~Transform()
{
}

void Transform::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
	isMatrixChanged = true;
}

void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = XMFLOAT3(position.x, position.y, position.z);
	isMatrixChanged = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	pitchYawRoll = XMFLOAT3(pitch, yaw, roll);
	isMatrixChanged = true;
	isRotationChanged = true;
}

void Transform::SetRotation(DirectX::XMFLOAT3 rotation)
{
	pitchYawRoll = XMFLOAT3(rotation.x, rotation.y, rotation.z);
	isMatrixChanged = true;
	isRotationChanged = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
	isMatrixChanged = true;
}

void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	this->scale = XMFLOAT3(scale.x, scale.y, scale.z);
	isMatrixChanged = true;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return pitchYawRoll;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if (isMatrixChanged) 
	{
		XMMATRIX t = XMMatrixTranslation(position.x, position.y, position.z);
		XMMATRIX s = XMMatrixScaling(scale.x, scale.y, scale.z);
		XMMATRIX r = XMMatrixRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);

		XMMATRIX worldMatrix = XMMatrixMultiply(XMMatrixMultiply(s, r), t);
		XMStoreFloat4x4(&world, worldMatrix);
		XMStoreFloat4x4(&worldInverseTranspose, XMMatrixInverse(0, XMMatrixTranspose(worldMatrix)));
		
		isMatrixChanged = false;
	}
	
	return world;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	GetWorldMatrix();
	
	return worldInverseTranspose;
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	if (isRotationChanged)
	{
		XMVECTOR workspace = XMQuaternionRotationRollPitchYaw(this->pitchYawRoll.x, this->pitchYawRoll.y, this->pitchYawRoll.z);
		workspace = XMVector3Rotate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), workspace);
		XMStoreFloat3(&right, workspace);

		isRotationChanged = false;
	}
	
	return right;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	if (isRotationChanged)
	{
		XMVECTOR workspace = XMQuaternionRotationRollPitchYaw(this->pitchYawRoll.x, this->pitchYawRoll.y, this->pitchYawRoll.z);
		workspace = XMVector3Rotate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), workspace);
		XMStoreFloat3(&up, workspace);

		isRotationChanged = false;
	}

	return up;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	if (isRotationChanged)
	{
		XMVECTOR workspace = XMQuaternionRotationRollPitchYaw(this->pitchYawRoll.x, this->pitchYawRoll.y, this->pitchYawRoll.z);
		workspace = XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), workspace);
		XMStoreFloat3(&forward, workspace);

		isRotationChanged = false;
	}

	return forward;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	position = XMFLOAT3(position.x + x, position.y + y, position.z + z);
	isMatrixChanged = true;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	position = XMFLOAT3(position.x + offset.x, position.y + offset.y, position.z + offset.z);
	isMatrixChanged = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
	XMVECTOR workspace = XMQuaternionRotationRollPitchYaw(this->pitchYawRoll.x, this->pitchYawRoll.y, this->pitchYawRoll.z);
	workspace = XMVector3Rotate(DirectX::XMVectorSet(x, y, z, 0.0f), workspace);
	XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), workspace));
	isMatrixChanged = true;
}

void Transform::MoveRelative(DirectX::XMFLOAT3 offset)
{
	XMVECTOR workspace = XMQuaternionRotationRollPitchYaw(this->pitchYawRoll.x, this->pitchYawRoll.y, this->pitchYawRoll.z);
	workspace = XMVector3Rotate(DirectX::XMVectorSet(offset.x, offset.y, offset.z, 0.0f), workspace);
	XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), workspace));
	isMatrixChanged = true;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	pitchYawRoll = XMFLOAT3(pitchYawRoll.x + pitch, pitchYawRoll.y + yaw, pitchYawRoll.z + roll);
	isMatrixChanged = true;
	isRotationChanged = true;
}

void Transform::Rotate(DirectX::XMFLOAT3 rotation)
{
	pitchYawRoll = XMFLOAT3(pitchYawRoll.x + rotation.x, pitchYawRoll.y + rotation.y, pitchYawRoll.z + rotation.z);
	isMatrixChanged = true;
	isRotationChanged = true;
}

void Transform::Scale(float x, float y, float z)
{
	scale = XMFLOAT3(scale.x * x, scale.y * y, scale.z * z);
	isMatrixChanged = true;
}

void Transform::Scale(DirectX::XMFLOAT3 scale)
{
	this->scale = XMFLOAT3(this->scale.x * scale.x, this->scale.y * scale.y, this->scale.z * scale.z);
	isMatrixChanged = true;
}
