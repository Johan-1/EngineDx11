#pragma once
#include "IComponent.h"
#include <DirectXMath.h>

using namespace DirectX;

class Entity;
class ModelComponent;

class UVScrollComponent : public IComponent
{
public:
	UVScrollComponent();
	~UVScrollComponent();

	void Init(XMFLOAT2 speed);
	void Update(const float& delta);

private:

	// pointer to model component
	ModelComponent* _modelComponent;

	// scrollspeed and current offset
	XMFLOAT2 _scrollSpeed;
	XMFLOAT2 _uvOffset;
};

