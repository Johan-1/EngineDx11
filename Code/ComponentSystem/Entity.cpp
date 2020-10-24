#include "Entity.h"
#include "IComponent.h"
#include "World.h"
#include "vectorHelpers.h"
#include "Systems.h"

// adds this entity to the world on creation
Entity::Entity()
{
	Systems::world->AddEntity(this);
}

Entity::~Entity()
{
}

// removes a component from the component list in entity
// aswell as in the component list in world
void Entity::RemoveComponent(IComponent* component) 
{
	VECTOR_HELPERS::RemoveItemFromVector(_components, component);
	Systems::world->RemoveComponent(component);
}

// removes all components owned by this entity from world
// aswell as removing the entity from entity list in world
void Entity::RemoveEntity() 
{
	World& world = *Systems::world;

	for (int i = 0; i < _components.size(); i++)
		world.RemoveComponent(_components[i]);

	world.RemoveEntity(this);
}