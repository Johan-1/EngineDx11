#pragma once

class SkyDome;

class IslandTestScene
{
public:
	IslandTestScene();
	~IslandTestScene();

	void Update(const float& delta);

private:

	SkyDome* _skyDome;
};

