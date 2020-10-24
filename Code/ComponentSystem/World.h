#pragma once
#include "IComponent.h"
#include <vector>

class Entity;
class IComponent;

// simple entity/component handler created for 
// easy use. a good idea whould be to change this 
// system to a more data oriented approch
class World
{
public:
	World();
	~World();

	void Update();
		
	// add component/entity
	void AddComponent(COMPONENT_TYPE type, IComponent* component) { _components[type].push_back(component); }
	void AddEntity(Entity* entity)                                { _entities.push_back(entity); }
	
	// add component/entity to dirty list
	void RemoveComponent(IComponent* component) { _componentsToRemove.push_back(component); }
	void RemoveEntity(Entity* entity)           { _entitiesToRemove.push_back(entity); }

	// will clear the world of all components and entities
	void Empty();

private:

	// add components/enteties that got flagged for
	// removal during the update function to these lists
	void HandleDirtyComponents();
	void HandleDirtyEntities();

	// deletes and removes the components/entities
	void DeleteComponent(COMPONENT_TYPE type, IComponent* component);
	void DeleteEntity(Entity* entity);

	// active components and entities
	std::vector<IComponent*> _components[NUM_COMPONENT_TYPES];
	std::vector<Entity*> _entities;

	// components/entities flagged for removal
	std::vector<IComponent*> _componentsToRemove;
	std::vector<Entity*>     _entitiesToRemove;
};

