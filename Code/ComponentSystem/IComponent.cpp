#include "IComponent.h"
#include "World.h"
#include "Systems.h"

// add this component to world by type
IComponent::IComponent(COMPONENT_TYPE type):
	_type(type),
	_isActive(true)
{
	Systems::world->AddComponent(type, this);
}

IComponent::~IComponent()
{	
}
