#pragma once

#include "Input.h"
#include <DirectXMath.h>
#include "Transform.h"
#include <memory>

class Camera
{
public:
	Camera(float aspectRatio, DirectX::XMFLOAT3 position, float fov, float nearPlane,
		float farPlane, float moveSpeed, float sensitivity, bool isOrthographic);
	~Camera();

	//Setters
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();

	//Getters
	std::shared_ptr<Transform> GetTransform();
	float GetFOV();
	float GetNearPlane();
	float GetFarPlane();
	bool GetIsOrthographic();

	//Update
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void Update(float dt);
	
private:
	std::shared_ptr<Transform> transform;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	float fov;
	float nearPlane;
	float farPlane;
	float moveSpeed;
	float sensitivtiy;
	bool isOrthographic;
};

