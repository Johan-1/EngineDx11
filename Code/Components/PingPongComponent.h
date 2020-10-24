#pragma once
#include "IComponent.h"
#include <DirectXMath.h>

using namespace DirectX;

class TransformComponent;

class PingPongComponent : public IComponent
{
public:
	PingPongComponent();
	virtual ~PingPongComponent();

	virtual void Update(const float& delta);
	void Init(XMFLOAT3 offsetPos, float offsetSpeed, XMFLOAT3 offsetRot = XMFLOAT3(0, 0, 0), XMFLOAT3 offsetSca = XMFLOAT3(0, 0, 0));

	XMFLOAT3 offsetPosition;
	XMFLOAT3 offsetRotation;
	XMFLOAT3 offsetScale;
	float speed;

private:

	TransformComponent* _transform;
	float _timer;
	float _wave;

	XMFLOAT3 _initialPosition;
	XMFLOAT3 _initialRotation;
	XMFLOAT3 _initialScale;
};

