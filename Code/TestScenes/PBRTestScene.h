#pragma once

class SkyDome;
class PBRTestScene
{
public:
	PBRTestScene();
	~PBRTestScene();

	void Update();

private:
	SkyDome* _skyDome;
};
