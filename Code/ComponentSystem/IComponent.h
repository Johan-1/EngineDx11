#pragma once
#include "ComponentTypes.h"
#include "Entity.h"

// base class for all components
// have a pure virtual update and a pointer to 
// the entity for talking to other components
// this system is written with convenience in mind but
// should be rewritten at some point with a data oriented approch
class IComponent
{
public:
	IComponent(COMPONENT_TYPE type);
	virtual ~IComponent();

	// everyone have thier own update defined
	virtual void Update(const float& delta) = 0;

	// inactivate this component meaning update wont be called
	// can overide this from each component if some more functionality is needed 
	// to be set before/after the set active call
	// example of this is that the model components remove/add its meshes to/from the renderer
	virtual void SetActive(bool active) { _isActive = active; }

	// is this component active
	// keep this data private so the SetActive functions have to be used
	const bool& IsActive() { return _isActive; }

	// get the type of component this is
	COMPONENT_TYPE Type() { return _type; }

	// convenience method to get another component without having to type parent first
	template <class T> T* GetComponent(){ return parent->GetComponent<T>(); }	

	// interface to parent and siblings
	Entity* parent;

protected:

	// type and pointer to entity
	COMPONENT_TYPE _type;
	bool           _isActive;
};

