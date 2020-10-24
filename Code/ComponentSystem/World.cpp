#include "World.h"
#include "Entity.h"
#include "IComponent.h"
#include "VectorHelpers.h"
#include "Systems.h"

World::World()
{
}

World::~World()
{
	Empty();
}

void World::Update()
{
	const float& delta = Systems::time->GetDeltaTime();

	// update all components one type at the time
	for (int i = 0; i < NUM_COMPONENT_TYPES; i++) 
	{
		size_t size = _components[i].size();
		for (int y = 0; y < size; y++)
			if (_components[i][y]->IsActive())
				_components[i][y]->Update(delta);
	}

	// remove and delete all components/entities that got flagged 
	// for removal during the update
	HandleDirtyComponents();	
	HandleDirtyEntities();	
}

// removes and deletes a component from the active component list
void World::DeleteComponent(COMPONENT_TYPE type, IComponent* component)
{
	VECTOR_HELPERS::RemoveItemFromVector(_components[type], component);
	delete component;
}

// removes and deletes an entity from the active entity list
void World::DeleteEntity(Entity* entity)
{
	VECTOR_HELPERS::RemoveItemFromVector(_entities, entity);
	delete entity;	
}

// check if we have any components flagged for removal
void World::HandleDirtyComponents() 
{
	size_t count = _componentsToRemove.size();
	if (count == 0)
		return;

	// delete all components that is flagged dirty
	for (int i = 0; i < count; i++)			
		DeleteComponent(_componentsToRemove[i]->Type(), _componentsToRemove[i]);

	// clear list
	_componentsToRemove.clear();
}

// check if we have any entities flagged for removal
void World::HandleDirtyEntities()
{
	size_t count = _entitiesToRemove.size();
	if (count == 0)
		return;

	// delete and remove from list
	for (int i = 0; i < count; i++)
		DeleteEntity(_entitiesToRemove[i]);

	// clear list
	_entitiesToRemove.clear();
}

// clear the world of all entities and components
void World::Empty() 
{
	// delete all components and entitys in the world
	for (int i = 0; i < NUM_COMPONENT_TYPES; i++) 
	{
		size_t size = _components[i].size();
		for (int y = 0; y < size; y++)
			delete _components[i][y];

		// clear list of this component type
		_components[i].clear();
	}

	size_t size = _entities.size();
	for (int i = 0; i < size; i++)
		delete _entities[i];

	// clear all lists
	_entities.clear();
	_entitiesToRemove.clear();
	_componentsToRemove.clear();
}