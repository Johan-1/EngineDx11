#include "UVScrollComponent.h"
#include "Entity.h"
#include "ModelComponent.h"
#include "ComponentTypes.h"
#include "Time.h"
#include "Mesh.h"
#include "Systems.h"

UVScrollComponent::UVScrollComponent() : IComponent(COMPONENT_TYPE::UVSCROLL_COMPONENT)
{
}

UVScrollComponent::~UVScrollComponent()
{
}

void UVScrollComponent::Init(XMFLOAT2 speed)
{
	_uvOffset    = XMFLOAT2(0, 0);
	_scrollSpeed = speed;

	_modelComponent = GetComponent<ModelComponent>();
}

void UVScrollComponent::Update(const float& delta)
{
	// add to offset
	_uvOffset.x += _scrollSpeed.x * delta;
	_uvOffset.y += _scrollSpeed.y * delta;

	// get all meshes in model
	const std::vector<Mesh*>& meshes = _modelComponent->meshes; 

	// set the offset on all meshes
	size_t count = meshes.size();
	for (int i = 0; i < count; i++)
		meshes[i]->uvOffset = _uvOffset;
}