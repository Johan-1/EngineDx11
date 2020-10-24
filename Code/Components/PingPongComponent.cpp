#include "PingPongComponent.h"
#include "Systems.h"
#include "World.h"
#include "TransformComponent.h"

PingPongComponent::PingPongComponent() : IComponent(PING_PONG_COMPONENT),
	_timer(0),
	_wave(0)
{
}

PingPongComponent::~PingPongComponent()
{
}

void PingPongComponent::Init(XMFLOAT3 offsetPos, float offsetSpeed, XMFLOAT3 offsetRot, XMFLOAT3 offsetSca)
{
	offsetPosition = offsetPos;
	offsetRotation = offsetRot;
	offsetScale    = offsetSca;
	speed          = offsetSpeed;

	_transform = GetComponent<TransformComponent>();

	_initialPosition = _transform->position;
	_initialRotation = _transform->rotation;
	_initialScale    = _transform->scale;
}

void PingPongComponent::Update(const float& delta)
{
	_timer += speed * delta;
	_wave   = sin(_timer);

	XMFLOAT3 wave3(_wave, _wave, _wave);

	XMStoreFloat3(&_transform->position, XMVectorAdd(XMLoadFloat3(&_initialPosition),
		XMVectorMultiply(XMLoadFloat3(&offsetPosition), XMLoadFloat3(&wave3))));

	XMStoreFloat3(&_transform->rotation, XMVectorAdd(XMLoadFloat3(&_initialRotation),
		XMVectorMultiply(XMLoadFloat3(&offsetRotation), XMLoadFloat3(&wave3))));

	XMStoreFloat3(&_transform->scale, XMVectorAdd(XMLoadFloat3(&_initialScale),
		XMVectorMultiply(XMLoadFloat3(&offsetScale), XMLoadFloat3(&wave3))));

	_transform->BuildWorldMatrix();
}