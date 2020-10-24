#include "FreeMoveComponent.h"
#include <DirectXMath.h>
#include "input.h"
#include "Time.h"
#include "TransformComponent.h"
#include "Gamepad.h"
#include "SystemDefs.h"
#include "Systems.h"

FreeMoveComponent::FreeMoveComponent() : IComponent(COMPONENT_TYPE::FREE_MOVE_COMPONENT)
{
}

FreeMoveComponent::~FreeMoveComponent()
{
}

void FreeMoveComponent::init(float movementSpeed, float rotationSpeed, float shiftSpeedMultiplier)
{	
	_moveSpeed            = movementSpeed;
	_rotationSpeed        = rotationSpeed;
	_shiftSpeedMultiplier = shiftSpeedMultiplier;
	_transform            = GetComponent<TransformComponent>();
}

void FreeMoveComponent::Update(const float& delta)
{	
	UpdateMovement(delta);		
}

void FreeMoveComponent::UpdateMovement(const float& deltaTime)
{	
	// get access to input
	Input& input = *Systems::input;

	// get all axises of our transform
	XMFLOAT3 forward, right, up;
	_transform->GetAllAxis(forward, right, up);
	
	XMFLOAT3 translation(0, 0, 0);

	float speedMultiplier = 1.0f;

	if (input.IsKeyHeld(DIK_LSHIFT))
		speedMultiplier = _shiftSpeedMultiplier;

	// get how much we should move
	float moveAmount = _moveSpeed * speedMultiplier * deltaTime;

	// get mouse movement
	float mouseX = input.GetMouseX();
	float mouseY = input.GetMouseY();

	XMFLOAT2 keyInput(0,0);
	
	// get keyboard input values
	if (input.IsKeyHeld(DIK_A))	keyInput.x -= 1.0f;
	if (input.IsKeyHeld(DIK_D)) keyInput.x += 1.0f;
	if (input.IsKeyHeld(DIK_S)) keyInput.y -= 1.0f;
	if (input.IsKeyHeld(DIK_W)) keyInput.y += 1.0f;	

	// input input and move values to vectors
	XMFLOAT3 direction(0,0,0); 
	XMFLOAT3 xInput(keyInput.x, keyInput.x, keyInput.x);
	XMFLOAT3 yInput(keyInput.y, keyInput.y, keyInput.y);
	XMFLOAT3 movement(moveAmount, moveAmount, moveAmount);

	// calculate the move direction from input and facing direction
	XMStoreFloat3(&direction,
		XMVector3Normalize(
		XMVectorAdd(
			XMVectorMultiply(XMLoadFloat3(&right),   XMLoadFloat3(&xInput)),
			XMVectorMultiply(XMLoadFloat3(&forward), XMLoadFloat3(&yInput)))));
	
	// get translation amount based on direction and how much we should move
	XMStoreFloat3(&translation, XMVectorMultiply(XMLoadFloat3(&direction), XMLoadFloat3(&movement)));
	
	// add rotation and translation to transform
	_transform->AddRotation(XMFLOAT3(mouseY * _rotationSpeed, mouseX * _rotationSpeed, 0));
	_transform->AddTranslation(translation);

	// calculate new world matrix
	_transform->BuildWorldMatrix();
}


