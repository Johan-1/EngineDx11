#include "LightDirectionComponent.h"
#include "LightManager.h"
#include "Systems.h"

LightDirectionComponent::LightDirectionComponent() : IComponent(COMPONENT_TYPE::DIR_LIGHT_COMPONENT)
{
}

// removes this light as the active one when this component is destroyed
LightDirectionComponent::~LightDirectionComponent()
{
	if (Systems::lightManager->directionalLight == this)
		Systems::lightManager->directionalLight = nullptr;
}

void LightDirectionComponent::Init(XMFLOAT4 lightColor, TransformComponent* transformOverride)
{
	// set light properties
	this->lightColor = lightColor;

	// get pointer to transform
	_transform = GetComponent<TransformComponent>();

	_overrideTransform = transformOverride;

	// set this light to be used for rendering
	Systems::lightManager->directionalLight = this;
}

XMFLOAT3 LightDirectionComponent::GetLightDirection()
{
	if (_overrideTransform != nullptr)
		return _overrideTransform->GetForward();

	return _transform->GetForward();
}

XMFLOAT3 LightDirectionComponent::GetLightDirectionInv()
{ 
	XMFLOAT3 result; 
	XMStoreFloat3(&result, XMVectorNegate(XMLoadFloat3(&GetLightDirection())));

	return result;
}

void LightDirectionComponent::Update(const float& delta)
{
}
