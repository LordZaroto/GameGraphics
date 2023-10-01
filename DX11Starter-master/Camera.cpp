#include "Camera.h"

// For the DirectX Math library
using namespace DirectX;

Camera::Camera(float aspectRatio, DirectX::XMFLOAT3 position, float fov, float nearPlane, float farPlane,
    float moveSpeed, float sensitivity, bool isOrthographic)
{
    this->fov = fov;
    this->nearPlane = nearPlane;
    this->farPlane = farPlane;
    this->moveSpeed = moveSpeed;
    this->sensitivtiy = sensitivity;
    this->isOrthographic = isOrthographic;

    transform = std::make_shared<Transform>();
    transform->SetPosition(position);

    UpdateProjectionMatrix(aspectRatio);
    UpdateViewMatrix();
}

Camera::~Camera()
{
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
    return view;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix()
{
    return projection;
}

std::shared_ptr<Transform> Camera::GetTransform()
{
    return transform;
}

float Camera::GetFOV()
{
    return fov;
}

float Camera::GetNearPlane()
{
    return nearPlane;
}

float Camera::GetFarPlane()
{
    return farPlane;
}

bool Camera::GetIsOrthographic()
{
    return isOrthographic;
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
    XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane));
}

void Camera::UpdateViewMatrix()
{
    XMFLOAT3 posVec = transform->GetPosition();
    XMFLOAT3 forwardVec = transform->GetForward();
    XMStoreFloat4x4(&view, XMMatrixLookToLH( XMVectorSet(posVec.x, posVec.y, posVec.z, 0.0f),
                                             XMVectorSet(forwardVec.x, forwardVec.y, forwardVec.z, 0.0f),
                                             XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
}

void Camera::Update(float dt)
{
    Input& input = Input::GetInstance();

    //Movement
    if (input.KeyDown('W'))
    {
        transform->MoveRelative(0.0f, 0.0f, 1.0f * moveSpeed * dt);
    }
    if (input.KeyDown('S'))
    {
        transform->MoveRelative(0.0f, 0.0f, -1.0f * moveSpeed * dt);
    }
    if (input.KeyDown('A'))
    {
        transform->MoveRelative(-1.0f * moveSpeed * dt, 0.0f, 0.0f);
    }
    if (input.KeyDown('D'))
    {
        transform->MoveRelative(1.0f * moveSpeed * dt, 0.0f, 0.0f);
    }
    if (input.KeyDown(VK_SPACE))
    {
        transform->MoveAbsolute(0.0f, 1.0f * moveSpeed * dt, 0.0f);
    }
    if (input.KeyDown(VK_SHIFT))
    {
        transform->MoveAbsolute(0.0f, -1.0f * moveSpeed * dt, 0.0f);
    }

    //Looking
    if (input.MouseLeftDown())
    {
        int cursorMovementX = input.GetMouseXDelta() * sensitivtiy;
        int cursorMovementY = input.GetMouseYDelta() * sensitivtiy;

        transform->Rotate(cursorMovementY, cursorMovementX, 0.0f);

        //Clamp X Rotation
        XMFLOAT3 rotVec = transform->GetPitchYawRoll();
        if (rotVec.x > XM_1DIVPI)
        {
            transform->SetRotation(XM_1DIVPI, rotVec.y, rotVec.z);
        }
        else if (rotVec.x < -XM_1DIVPI)
        {
            transform->SetRotation(-XM_1DIVPI, rotVec.y, rotVec.z);
        }
    }

    UpdateViewMatrix();
}
