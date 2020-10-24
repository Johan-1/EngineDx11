#include "LightManager.h"
#include "ShaderHelpers.h"

LightManager::LightManager():
	cbPoint(nullptr),
	cbAmbDir(nullptr)
{	
	SHADER_HELPERS::CreateConstantBuffer(cbPoint);
	SHADER_HELPERS::CreateConstantBuffer(cbAmbDir);
}

LightManager::~LightManager()
{
}

// add a new point light
void LightManager::AddPointLight(LightPointComponent* light)
{
	if (_pointLights.size() < MAX_POINT_LIGHTS)
	{
		_pointLights.push_back(light);
		return;
	}

	// delete entity and all its components if we have more lights then
	// our shader is set to be able to recive in one drawcall
	light->parent->RemoveEntity();
	printf("Could not add light, alredy at maximum\n");
}

void LightManager::UpdateLightBuffers()
{
	// set the pointlight data
	size_t size = _pointLights.size();
	for (int i = 0; i < size; i++)
	{
		_pointData[i].color          = _pointLights[i]->color;
		_pointData[i].intensity      = _pointLights[i]->intensity;
		_pointData[i].radius         = _pointLights[i]->radius;
		_pointData[i].lightPosition  = _pointLights[i]->GetComponent<TransformComponent>()->position;
		_pointData[i].attConstant    = _pointLights[i]->attConstant;
		_pointData[i].attLinear      = _pointLights[i]->attLinear;
		_pointData[i].attExponential = _pointLights[i]->attExponential;
		_pointData[i].numLights      = size;
	}

	// set the ambient/directional light data
	XMStoreFloat4(&_ambDirData.ambientColor, XMLoadFloat4(&ambientColor));
	XMStoreFloat4(&_ambDirData.dirColor,     XMLoadFloat4(&directionalLight->lightColor));
	XMStoreFloat3(&_ambDirData.lightDir,     XMLoadFloat3(&directionalLight->GetLightDirectionInv()));

	// update the buffers
	SHADER_HELPERS::UpdateConstantBuffer(&_ambDirData, sizeof(CBAmbDir),       cbAmbDir);
	SHADER_HELPERS::UpdateConstantBuffer(&_pointData,  sizeof(CBPoint) * size, cbPoint);
}