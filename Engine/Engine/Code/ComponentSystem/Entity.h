#pragma once
#include "ComponentTypes.h"
#include <vector>

class IComponent;
class ModelComponent;
class RotationComponent;
class FreeMoveComponent;
class CameraComponent;
class QuadComponent;
class UVScrollComponent;
class ParticleSystemComponent;
class LightDirectionComponent;
class LightPointComponent;
class PingPongComponent;

class Entity
{
public:
	Entity();
	~Entity();	

	// create component and add to components list
	template <class T> T* AddComponent()
	{
		T* component = new T();

		// we asume that T inherits from IComponent (witch we know) 
		// so we can call SetParent 
		component->parent = this;
		_components.push_back(component);
		
		return component;
	}

	// gets a component by T type
	template <class T> T* GetComponent()
	{
		// get the enum type from templete so we can use that as key
		// to find the componnet we are asking for
		COMPONENT_TYPE type = GetTypeFromTemplate<T>();

		// return the component that match the enum and cast it
		// to the type we requested
		for (int i = 0; i < _components.size(); i++)		
			if (_components[i]->Type() == type)			
				return (T*)_components[i];
		
		// return null if we don't have the component
		return nullptr;
	}

	void RemoveComponent(IComponent* component);
	void RemoveEntity();

private:

	// list of all components of this entity
	std::vector<IComponent*> _components;

	// a kind of dirty function for retriving the enum ID of a component from T type
	template <class T> COMPONENT_TYPE GetTypeFromTemplate()
	{
		if (std::is_same<T, TransformComponent>::value)       { return TRANSFORM_COMPONENT; }													   
		if (std::is_same<T, ModelComponent>::value)           { return MODEL_COMPONENT; }															   
		if (std::is_same<T, RotationComponent>::value)        { return TRANSFORMATION_COMPONENT; }													   
		if (std::is_same<T, FreeMoveComponent>::value)        { return FREE_MOVE_COMPONENT; }													   
		if (std::is_same<T, CameraComponent>::value)          { return CAMERA_COMPONENT; }													   
		if (std::is_same<T, QuadComponent>::value)            { return QUAD_COMPONENT; }													   
		if (std::is_same<T, LightDirectionComponent>::value)  { return DIR_LIGHT_COMPONENT; }													   
		if (std::is_same<T, LightPointComponent>::value)      { return POINT_LIGHT_COMPONENT; }													   
		if (std::is_same<T, UVScrollComponent>::value)        { return UVSCROLL_COMPONENT; }
		if (std::is_same<T, ParticleSystemComponent>::value)  { return PARTICLE_COMPONENT; }
		if (std::is_same<T, PingPongComponent>::value)        { return PING_PONG_COMPONENT; }

		printf("Failed to get Component enum ID from T template\n");

		// just return this in case nothing matched the T
		// should only get here if a newly created component type have not been added to this function
		return NUM_COMPONENT_TYPES;		
	}

};

