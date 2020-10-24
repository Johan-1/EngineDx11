#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "LightDirectionComponent.h"
#include "LightPointComponent.h"

using namespace DirectX;

#define MAX_POINT_LIGHTS 1024

// constant data for point lightning in the lightning pass
struct CBPoint
{
	XMFLOAT3 lightPosition;
	float    radius;
	XMFLOAT3 color;
	float    intensity;
	float attConstant;
	float attLinear;
	float attExponential;
	int   numLights;
};

struct CBAmbDir
{
	XMFLOAT4 ambientColor;
	XMFLOAT4 dirColor;
	XMFLOAT3 lightDir;
	float    pad;
};

class LightManager
{
public:
	LightManager();
	~LightManager();				

	// add a pointlight to the list of pointlights
	// list of pointlights is kept private so it have
	// to go throught the AddPointLight function to make
	// sure we dont add more point lights then the shader can 
	// fit in one buffer
	void AddPointLight(LightPointComponent* light);
	const std::vector<LightPointComponent*>& GetPointLights() { return _pointLights; }
	int GetNumPointLights()                                   { return (int)_pointLights.size(); }
		
	// updates the data for light constant buffers
	void UpdateLightBuffers();


	// lights
	LightDirectionComponent* directionalLight;
	XMFLOAT4                 ambientColor;

	// light CB buffers
	ID3D11Buffer* cbPoint;
	ID3D11Buffer* cbAmbDir;
	
private:	

	// holds the properties of all point lights
	CBPoint       _pointData[MAX_POINT_LIGHTS];
	CBAmbDir      _ambDirData;

	// point lights
	std::vector<LightPointComponent*> _pointLights;
};

