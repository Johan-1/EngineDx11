#include "CameraComponent.h"
#include "SystemDefs.h"
#include "TransformComponent.h"
#include "MathHelpers.h"

CameraComponent::CameraComponent() : IComponent(COMPONENT_TYPE::CAMERA_COMPONENT)
{
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::Init3D(const float& fov)
{
	// get ptr to transform component
	_transform = GetComponent<TransformComponent>();

	// Create the projection matrix for 3D rendering.	
	XMStoreFloat4x4(&projectionMatrix, XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), SystemSettings::SCREEN_WIDTH / SystemSettings::SCREEN_HEIGHT, 0.1f, 5000.0f));

	CalculateViewMatrix();
}

void CameraComponent::Init2D(const XMFLOAT2& size, const XMFLOAT2& nearfar)
{
	// get ptr to transform component
	_transform = GetComponent<TransformComponent>();

	// create ortho projection
	XMStoreFloat4x4(&projectionMatrix, XMMatrixOrthographicLH(size.x, size.y, nearfar.x, nearfar.y));

	CalculateViewMatrix();
}

void CameraComponent::Update(const float& delta)
{		
	CalculateViewMatrix();	
}

// calculates the view matrix from the position, forward and up vectors
void CameraComponent::CalculateViewMatrix() 
{
	XMFLOAT3 forward, right, up;
	_transform->GetAllAxis(forward, right, up);
	
	// create the viewMatrix based on our position, look direction and updirection of the camera	
	XMStoreFloat3(&forward, XMVectorAdd(XMLoadFloat3(&_transform->position), XMLoadFloat3(&forward)));

	// create matrices
	XMStoreFloat4x4(&viewMatrix,          XMMatrixLookAtLH(XMLoadFloat3(&_transform->position), XMLoadFloat3(&forward), XMLoadFloat3(&up)));
	XMStoreFloat4x4(&viewProjMatrix,      XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiply(&viewMatrix, &projectionMatrix)));
	XMStoreFloat4x4(&viewProjMatrixTrans, XMMatrixTranspose(XMLoadFloat4x4(&viewProjMatrix)));
}
