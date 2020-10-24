#pragma once
#include <vector>
class InstancedModel;
class SkyDome;
class SponzaTestScene

{
public:
	SponzaTestScene();
	~SponzaTestScene();

	void Update();

private:

	SkyDome* _skyDome;
	InstancedModel* _instancedSpheres;
	float _sineWaves[2];
	float _sineTimers[2];
};

