#include "SkyDome.h"
#include "CameraManager.h"
#include "DXManager.h"
#include "TransformComponent.h"
#include "ThirdParty/DDSTextureLoader/DDSTextureLoader.h"
#include "DXErrorHandler.h"
#include "ShaderHelpers.h"
#include "DXBlendstates.h"
#include "DXRasterizerStates.h"
#include "DXDepthStencilStates.h"
#include "Systems.h"
#include "ModelLoader.h"
#include "Mesh.h"
#include <algorithm>
#include "MathHelpers.h"
#include "Entity.h"
#include "JsonHelpers.h"

SkyDome::SkyDome(const char* settingsFile):
	isActive(true)
{
	// create mesh and cubemap
	CreateMeshes();

	// create shaders
	SHADER_HELPERS::CreateVertexShader(L"Data/shaders/SkyDome/vertexSkyDomeCubemap.shader",      _vertexDomeCubeMapShader,      vertexDomeCubeMapShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"Data/shaders/SkyDome/pixelSkyDomeCubemap.shader",        _pixelDomeCubeMapShader,       pixelDomeCubeMapShaderByteCode);
	SHADER_HELPERS::CreateVertexShader(L"Data/shaders/SkyDome/vertexSkyDomeColorBlend.shader",   _vertexDomeColorBlendShader,   vertexDomeColorBlendShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"Data/shaders/SkyDome/pixelSkyDomeColorBlend.shader",     _pixelDomeColorBlendShader,    pixelDomeColorBlendShaderByteCode);
	SHADER_HELPERS::CreateVertexShader(L"Data/shaders/SkyDome/vertexSkyDomeSun.shader",          _vertexSunShader,              vertexSunShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"Data/shaders/SkyDome/pixelSkyDomeSun.shader",            _pixelSunShader,               pixelSunShaderByteCode);

	// create constant buffers
	SHADER_HELPERS::CreateConstantBuffer(_constantBufferVertex);
	SHADER_HELPERS::CreateConstantBuffer(_constantBufferColorBlendPixel);
	SHADER_HELPERS::CreateConstantBuffer(_constantBufferCubeMapBlendPixel);
	SHADER_HELPERS::CreateConstantBuffer(_constantSunPixel);
	
	sunMoon.sun.entity = new Entity();	
 	sunMoon.sun.transform = sunMoon.sun.entity->AddComponent<TransformComponent>();
	sunMoon.sun.transform->Init();

	sunMoon.moon.entity = new Entity();
	sunMoon.moon.transform = sunMoon.moon.entity->AddComponent<TransformComponent>();
	sunMoon.moon.transform->Init();

	ReadSettings(settingsFile);
	LoadCubemap();
}

SkyDome::~SkyDome()
{
	vertexDomeCubeMapShaderByteCode->Release();
	pixelDomeCubeMapShaderByteCode->Release();
	vertexSunShaderByteCode->Release();
	pixelSunShaderByteCode->Release();

	_vertexDomeCubeMapShader->Release();
	_pixelDomeCubeMapShader->Release();
	_vertexSunShader->Release();
	_pixelSunShader->Release();

	_constantBufferVertex->Release();
	_constantBufferColorBlendPixel->Release();
	_constantSunPixel->Release();
	_constantBufferCubeMapBlendPixel->Release();
}

void SkyDome::CreateMeshes() 
{
	_domeMesh         = ModelLoader::CreateSphere(0, L"", L"", L"", L"", L"", 1.0f, nullptr, 0);
	sunMoon.sun.mesh  = ModelLoader::CreateWorldSprite(0, L"Data/Textures/sun.dds", nullptr);
	sunMoon.moon.mesh = ModelLoader::CreateWorldSprite(0, L"Data/Textures/moon2.dds", nullptr);
}

void SkyDome::LoadCubemap() 
{
	// remove the old cubemap texture if one exist
	if (skySettings.cubeMap != nullptr)
		skySettings.cubeMap->Release();

	std::wstring file(skySettings.cubeMapName.begin(), skySettings.cubeMapName.end());

	// create cubemap from file
	HRESULT result = DirectX::CreateDDSTextureFromFile(Systems::dxManager->device, file.c_str(), NULL, &skySettings.cubeMap);

	// get and print error message if failed
	if (FAILED(result))						
		DX_ERROR::PrintError(result, (std::string("failed to create cubemap with filename ") + DX_ERROR::ConvertFromWString(file.c_str())).c_str());
}

void SkyDome::Render(bool noMask)
{
	if (!isActive)
		return;

	if (skySettings.RENDER_MODE == SKY_DOME_RENDER_MODE::CUBEMAP) RenderCubeMap(noMask);
	else                                                           RenderBlendedColors(noMask);
	
	RenderSunMoon(noMask);	
}

void SkyDome::RenderCubeMap(bool noMask)
{
	// RENDER SKYDOME SPHERE
	// get DX manager
	DXManager& DXM = *Systems::dxManager;

	// get device context
	ID3D11DeviceContext*& devCon = DXM.devCon;

	// get game camera
	CameraComponent*& camera     = Systems::cameraManager->currentCameraGame;
	TransformComponent* camTrans = camera->GetComponent<TransformComponent>();

	// set shaders			
	devCon->VSSetShader(_vertexDomeCubeMapShader, NULL, 0);
	devCon->PSSetShader(_pixelDomeCubeMapShader, NULL, 0);

	// set constant buffer
	devCon->VSSetConstantBuffers(0, 1, &_constantBufferVertex);

	// set texture
	devCon->PSSetShaderResources(0, 1, &skySettings.cubeMap);

	// use no culling
	DXM.rasterizerStates->SetRasterizerState(RASTERIZER_STATE::NOCULL);

	if (noMask) DXM.depthStencilStates->SetDepthStencilState(DEPTH_STENCIL_STATE::DISABLED);
	else        DXM.depthStencilStates->SetDepthStencilState(DEPTH_STENCIL_STATE::MASKED_SKYBOX);
	
	CBVertDome vertexData;
	XMStoreFloat4x4(&vertexData.worldViewProj, XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&camTrans->positionMatrix, &camera->viewProjMatrix)));

	// update constant buffer
	SHADER_HELPERS::UpdateConstantBuffer((void*)&vertexData, sizeof(CBVertDome), _constantBufferVertex);

	// upload and draw dome mesh
	_domeMesh->UploadBuffers();
	devCon->DrawIndexed(_domeMesh->numIndices, 0, 0);

	ID3D11ShaderResourceView* nullSRV[1] = { NULL };
	devCon->PSSetShaderResources(0, 1, nullSRV);
}

void SkyDome::RenderBlendedColors(bool noMask)
{
	// RENDER SKYDOME SPHERE
	// get DX manager
	DXManager& DXM = *Systems::dxManager;

	// get device context
	ID3D11DeviceContext* devCon = DXM.devCon;

	// get game camera
	CameraComponent* camera      = Systems::cameraManager->currentCameraGame;
	TransformComponent* camTrans = camera->GetComponent<TransformComponent>();

	// set shaders			
	devCon->VSSetShader(_vertexDomeColorBlendShader, NULL, 0);
	devCon->PSSetShader(_pixelDomeColorBlendShader, NULL, 0);

	// set constant buffer
	devCon->VSSetConstantBuffers(0, 1, &_constantBufferVertex);
	devCon->PSSetConstantBuffers(0, 1, &_constantBufferColorBlendPixel);

	// use no culling
	DXM.rasterizerStates->SetRasterizerState(RASTERIZER_STATE::NOCULL);
	
	// set vertex constants
	if (noMask) DXM.depthStencilStates->SetDepthStencilState(DEPTH_STENCIL_STATE::DISABLED);
	else        DXM.depthStencilStates->SetDepthStencilState(DEPTH_STENCIL_STATE::MASKED_SKYBOX);
	
	CBVertDome vertexData;
	XMStoreFloat4x4(&vertexData.worldViewProj, XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&camTrans->positionMatrix, &camera->viewProjMatrix)));

	// set the colors, the percent fraction is stored in the w channel that specifies how
	// low to high a color will be used on the skydome 
	ConstantColorBlendPixel pixeldata;
	pixeldata.bottom   = skySettings.skyBottomColor;
	pixeldata.mid      = skySettings.skyMidColor;
	pixeldata.top      = skySettings.skyTopColor;

	// update constant buffers
	SHADER_HELPERS::UpdateConstantBuffer((void*)&vertexData, sizeof(CBVertDome),              _constantBufferVertex);
	SHADER_HELPERS::UpdateConstantBuffer((void*)&pixeldata,  sizeof(ConstantColorBlendPixel), _constantBufferColorBlendPixel);

	// upload and draw dome mesh
	_domeMesh->UploadBuffers();
	devCon->DrawIndexed(_domeMesh->numIndices, 0, 0);
}

void SkyDome::RenderSunMoon(bool noMask)
{
	// get DX manager
	DXManager& DXM = *Systems::dxManager;

	// get device context
	ID3D11DeviceContext* devCon = DXM.devCon;

	// get game camera
	CameraComponent* camera      = Systems::cameraManager->currentCameraGame;
	TransformComponent* camTrans = camera->GetComponent<TransformComponent>();

	// render sun with alpha blend
	DXM.blendStates->SetBlendState(BLEND_ALPHA);

	// set shaders			
	devCon->VSSetShader(_vertexSunShader, NULL, 0);
	devCon->PSSetShader(_pixelSunShader, NULL, 0);

	devCon->PSSetConstantBuffers(0, 1, &_constantSunPixel);

	// calculate sun/moon data
	CaluclateSunMoonMatrix(camTrans->position);

	// set world and projection matrix
	CBVertSun vertexData;
	XMStoreFloat4x4(&vertexData.world, XMLoadFloat4x4(&sunMoon.sun.positionMatrix));
	XMStoreFloat4x4(&vertexData.view, XMMatrixTranspose(XMLoadFloat4x4(&camera->viewMatrix)));
	XMStoreFloat4x4(&vertexData.Proj, XMMatrixTranspose(XMLoadFloat4x4(&camera->projectionMatrix)));

	// constantbuffer pixel structure
	ConstantSunPixel pixeldata;
	pixeldata.sunDot       = sunMoon.sun.relativeHeight;
	pixeldata.colorTint    = sunMoon.sun.colorTint;
	pixeldata.beginEndFade = sunMoon.sun.beginEndFade;

	// update constant buffer
	SHADER_HELPERS::UpdateConstantBuffer((void*)&vertexData, sizeof(CBVertSun),        _constantBufferVertex);
	SHADER_HELPERS::UpdateConstantBuffer((void*)&pixeldata,  sizeof(ConstantSunPixel), _constantSunPixel);

	// upload mesh
	sunMoon.sun.mesh->UploadBuffers();

	// set texture
	devCon->PSSetShaderResources(0, 1, &sunMoon.sun.mesh->baseTextures[0]);

	// upload and draw sun
	devCon->DrawIndexed(sunMoon.sun.mesh->numIndices, 0, 0);

	// change properties for moon
	vertexData.world       = sunMoon.moon.positionMatrix;
	pixeldata.sunDot       = sunMoon.moon.relativeHeight;
	pixeldata.colorTint    = sunMoon.moon.colorTint;
	pixeldata.beginEndFade = sunMoon.moon.beginEndFade;

	// update constant buffer
	SHADER_HELPERS::UpdateConstantBuffer((void*)&vertexData, sizeof(CBVertSun),       _constantBufferVertex);
	SHADER_HELPERS::UpdateConstantBuffer((void*)&pixeldata, sizeof(ConstantSunPixel), _constantSunPixel);

	// upload mesh
	sunMoon.moon.mesh->UploadBuffers();

	// set texture
	devCon->PSSetShaderResources(0, 1, &sunMoon.moon.mesh->baseTextures[0]);

	// upload and draw moon
	devCon->DrawIndexed(sunMoon.moon.mesh->numIndices, 0, 0);

	// enable rendering of all pixels
	DXM.depthStencilStates->SetDepthStencilState(DEPTH_STENCIL_STATE::ENABLED);

	// set back to backcull
	DXM.rasterizerStates->SetRasterizerState(RASTERIZER_STATE::BACKCULL);

	ID3D11ShaderResourceView* nullSRV[1] = { NULL };
	devCon->PSSetShaderResources(0, 1, nullSRV);
}

void SkyDome::CaluclateSunMoonMatrix(XMFLOAT3 cameraPosition)
{
	XMFLOAT3 offsetSun;
	XMFLOAT3 offsetMoon;
	XMFLOAT3 sunDirection  = sunMoon.sun.transform->GetForward();
	XMFLOAT3 moonDirection = sunMoon.moon.transform->GetForward();
	
	// calculate translation for sun and moon
	XMStoreFloat3(&offsetSun,  XMVectorMultiply(XMLoadFloat3(&sunDirection), XMLoadFloat3(&sunMoon.sun.distance)));
	XMStoreFloat3(&offsetMoon, XMVectorMultiply(XMLoadFloat3(&moonDirection), XMLoadFloat3(&sunMoon.moon.distance)));

	XMStoreFloat3(&offsetSun,  XMVectorSubtract(XMLoadFloat3(&cameraPosition), XMLoadFloat3(&offsetSun)));
	XMStoreFloat3(&offsetMoon, XMVectorSubtract(XMLoadFloat3(&cameraPosition), XMLoadFloat3(&offsetMoon)));

	// calculate position matrix for sun and moon
	XMStoreFloat4x4(&sunMoon.sun.positionMatrix,  XMMatrixTranslationFromVector(XMLoadFloat3(&offsetSun)));
	XMStoreFloat4x4(&sunMoon.sun.positionMatrix,  XMMatrixTranspose(XMLoadFloat4x4(&sunMoon.sun.positionMatrix)));

	XMStoreFloat4x4(&sunMoon.moon.positionMatrix, XMMatrixTranslationFromVector(XMLoadFloat3(&offsetMoon)));
	XMStoreFloat4x4(&sunMoon.moon.positionMatrix, XMMatrixTranspose(XMLoadFloat4x4(&sunMoon.moon.positionMatrix)));
}

void SkyDome::Update(const float& delta)
{
	UpdateSunMoonTranslation(delta);
	UpdateSunMoonColors(delta);
	UpdateShadowLightTranslation(delta);
	UpdateShadowLightColor(delta);

	if (skySettings.RENDER_MODE == THREE_LAYER_COLOR_BLEND) UpdateSkyColors(delta);
}

void SkyDome::UpdateSunMoonTranslation(const float& delta)
{
	// add to cycle timer and reset if we have passed one complete cycle
	skySettings.cycleTimer += delta * skySettings.speedMultiplier;
	if (skySettings.cycleTimer > skySettings.cycleInSec)
		skySettings.cycleTimer = 0.0f;

	XMFLOAT3 sunRotation;
	XMFLOAT3 moonRotation;
	XMFLOAT3 moonOffset(180.0f, 0, 0);

	// get the current rotation of the sun bassed on cycle passed
	skySettings.endRotation.y = skySettings.startRotation.y;
	XMStoreFloat3(&sunRotation,
		XMVectorLerp(XMLoadFloat3(&skySettings.startRotation),
			XMLoadFloat3(&skySettings.endRotation),
			skySettings.cycleTimer / skySettings.cycleInSec));

	// add on 180 degress to moon rotation from sun rotation
	XMStoreFloat3(&moonRotation, XMVectorAdd(XMLoadFloat3(&sunRotation), XMLoadFloat3(&moonOffset)));

	// update rotation and matrices
	sunMoon.sun.transform->rotation  = sunRotation;
	sunMoon.moon.transform->rotation = moonRotation;
	sunMoon.sun.transform->BuildWorldMatrix();
	sunMoon.moon.transform->BuildWorldMatrix();

	// get the inverse look of sun
	XMFLOAT3 invertedForward = sunMoon.sun.transform->GetForward();
	XMStoreFloat3(&invertedForward, XMVectorNegate(XMLoadFloat3(&invertedForward)));

	// get the dot product that will represent the sun at its highest point
	// 1.0 = top, 0.0 = at horizon, -1 = at lowest point under the world
	XMFLOAT3 up(0, 1, 0);
	XMStoreFloat(&sunMoon.sun.relativeHeight,
		XMVector3Dot(XMLoadFloat3(&invertedForward), XMLoadFloat3(&up)));

	sunMoon.moon.relativeHeight = -sunMoon.sun.relativeHeight;

	// set distance of sun/moon depending on thier respective relative height
	float dst = lerpF(sunMoon.sun.minMaxDst.y, sunMoon.sun.minMaxDst.x,
		inverseLerp(sunMoon.sun.beginEndDstLerp.x, sunMoon.sun.beginEndDstLerp.y, sunMoon.sun.relativeHeight));
	sunMoon.sun.distance = XMFLOAT3(dst, dst, dst);

	dst = lerpF(sunMoon.moon.minMaxDst.y, sunMoon.moon.minMaxDst.x,
		inverseLerp(sunMoon.moon.beginEndDstLerp.x, sunMoon.moon.beginEndDstLerp.y, sunMoon.moon.relativeHeight));
	sunMoon.moon.distance = XMFLOAT3(dst, dst, dst);
}

void SkyDome::UpdateSunMoonColors(const float& delta)
{
	// set color tint on sun/moon depending on thier own relative height
	LerpColorRGB(sunMoon.sun.colorTint, sunMoon.sun.dayColorTint, sunMoon.sun.sunsetColorTint,
		sunMoon.sun.beginEndColorBlend.x, sunMoon.sun.beginEndColorBlend.y, sunMoon.sun.relativeHeight);

	LerpColorRGB(sunMoon.moon.colorTint, sunMoon.moon.dayColorTint, sunMoon.moon.sunsetColorTint,
		sunMoon.moon.beginEndColorBlend.x, sunMoon.moon.beginEndColorBlend.y, sunMoon.moon.relativeHeight);
}

void SkyDome::UpdateShadowLightTranslation(const float& delta)
{
	// get the transform of the camera that renders the shadowmap, the directional light 
	// use this same transform for setting the light direction
	TransformComponent* lightTransform = Systems::cameraManager->currentCameraDepthMap->GetComponent<TransformComponent>();

	// set new light rotation depending on the threshold if
	// the sun or the moon is the light casting source
	lightTransform->rotation = sunMoon.sun.relativeHeight < skySettings.switchToMoonLightThreshold ? sunMoon.moon.transform->rotation : sunMoon.sun.transform->rotation;
	lightTransform->BuildWorldMatrix();

	// get the inverted forward of the current lightsource
	// so we can offset the shadowmap camera based on this
	XMFLOAT3 invertedForwardLight;
	XMStoreFloat3(&invertedForwardLight, XMVectorNegate(XMLoadFloat3(&lightTransform->GetForward())));

	// move the shadowmap camera 
	XMStoreFloat3(&lightTransform->position, XMVectorMultiply(XMLoadFloat3(&invertedForwardLight), XMLoadFloat3(&skySettings.shadowMapDistance)));
	lightTransform->BuildWorldMatrix();
}

void SkyDome::UpdateShadowLightColor(const float& delta)
{
	// get the directional light so we can change the colors of light
	LightDirectionComponent*& directionLight = Systems::lightManager->directionalLight;

	// final directional light color depending on the dot product
	// day to sunset
	XMFLOAT4 blendedLightColor;
	LerpColorRGB(blendedLightColor, skySettings.normalDirLightColor, skySettings.sunsetDirLightColor,
		skySettings.sunsetLightColorStartEndBlend.x, skySettings.sunsetLightColorStartEndBlend.y, sunMoon.sun.relativeHeight);

	// sunset to night
	LerpColorRGB(blendedLightColor, blendedLightColor, skySettings.nightDirLightColor,
		skySettings.nightLightColorStartEndBlend.x, skySettings.nightLightColorStartEndBlend.y, sunMoon.sun.relativeHeight);

	// get the light strength fraction depending on if night or day
	// this makes the light completely fade out before we switch between
	// the sun and moon light sources. After the switch the light strength is 
	// then faded back in, this avoids a rough light transition
	float ls = sunMoon.sun.relativeHeight < skySettings.switchToMoonLightThreshold ?
		inverseLerp(skySettings.nightLightStartEndfade.y, skySettings.nightLightStartEndfade.x, sunMoon.sun.relativeHeight) :
		inverseLerp(skySettings.dayLightStartEndfade.y,   skySettings.dayLightStartEndfade.x,   sunMoon.sun.relativeHeight);

	// mutiply light color by strenght
	XMFLOAT4 lightStrength(ls, ls, ls, 1.0f);
	XMStoreFloat4(&blendedLightColor, XMVectorMultiply(XMLoadFloat4(&blendedLightColor), XMLoadFloat4(&lightStrength)));

	// set the color of directional light
	directionLight->lightColor = blendedLightColor;
}

void SkyDome::UpdateSkyColors(const float& delta)
{
	// set the color of the top part of sky
	// blend between day color to sunsetcolor and then to nightcolor
	XMFLOAT4 topSkyColor;
	LerpColorRGB(topSkyColor, skySettings.topSkyColorDay, skySettings.topSkyColorSunSet,
		skySettings.sunsetTopSkyColorStartEndBlend.x,     skySettings.sunsetTopSkyColorStartEndBlend.y, sunMoon.sun.relativeHeight);

	LerpColorRGB(skySettings.skyTopColor, topSkyColor, skySettings.topSkyColorNight,
		skySettings.nightTopSkyColorStartEndBlend.x,    skySettings.nightTopSkyColorStartEndBlend.y, sunMoon.sun.relativeHeight);

	// set the color of the mid part of sky
	// blend between day color to sunsetcolor and then to nightcolor
	XMFLOAT4 midSkyColor;
	LerpColorRGB(midSkyColor, skySettings.midSkyColorDay, skySettings.midSkyColorSunSet,
		skySettings.sunsetMidSkyColorStartEndBlend.x, skySettings.sunsetMidSkyColorStartEndBlend.y, sunMoon.sun.relativeHeight);

	LerpColorRGB(skySettings.skyMidColor, midSkyColor, skySettings.midSkyColorNight,
		skySettings.nightMidSkyColorStartEndBlend.x, skySettings.nightMidSkyColorStartEndBlend.y, sunMoon.sun.relativeHeight);
}

float SkyDome::inverseLerp(float a, float b, float t)
{
	float result = ((t - a) / (b - a));

	return result < 0.0f ? 0.0f : result > 1.0f ? 1.0f : result;	
}

float SkyDome::lerpF(float a, float b, float f)
{
	return (a * (1.0f - f)) + (b * f);
}

void SkyDome::LerpColorRGB(XMFLOAT4& result, XMFLOAT4 colorA, XMFLOAT4 colorB, float startBlend, float endBlend, float fraction)
{
	XMFLOAT4 color;

	XMStoreFloat4(&color,
		XMVectorLerp(XMLoadFloat4(&colorA), XMLoadFloat4(&colorB), inverseLerp(startBlend, endBlend, fraction)));

	result.x = color.x;
	result.y = color.y;
	result.z = color.z;
}

void SkyDome::LerpColorRGB(XMFLOAT3& result, XMFLOAT3 colorA, XMFLOAT3 colorB, float startBlend, float endBlend, float fraction)
{
	XMStoreFloat3(&result,
		XMVectorLerp(XMLoadFloat3(&colorA), XMLoadFloat3(&colorB), inverseLerp(startBlend, endBlend, fraction)));
}

void SkyDome::ReadSettings(const char* file)
{
	FILE* fp; fopen_s(&fp, file, "rb");
	char readBuffer[65536];
	rapidjson::FileReadStream inStream(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document d;
	d.ParseStream(inStream);
	fclose(fp);

	assert(d.IsObject());

	skySettings.speedMultiplier                = JSON::ReadFloat(d, "speedMultiplier");
	skySettings.cycleTimer                     = JSON::ReadFloat(d, "cycleTimer");
	skySettings.switchToMoonLightThreshold     = JSON::ReadFloat(d, "switchToMoonLightThreshold");
	skySettings.sunsetLightColorStartEndBlend  = JSON::ReadFloat2(d, "sunsetLightColorStartEndBlend");
	skySettings.nightLightColorStartEndBlend   = JSON::ReadFloat2(d, "nightLightColorStartEndBlend");
	skySettings.sunsetTopSkyColorStartEndBlend = JSON::ReadFloat2(d, "sunsetTopSkyColorStartEndBlend");
	skySettings.nightTopSkyColorStartEndBlend  = JSON::ReadFloat2(d, "nightTopSkyColorStartEndBlend");
	skySettings.sunsetMidSkyColorStartEndBlend = JSON::ReadFloat2(d, "sunsetMidSkyColorStartEndBlend");
	skySettings.nightMidSkyColorStartEndBlend  = JSON::ReadFloat2(d, "nightMidSkyColorStartEndBlend");
	skySettings.dayLightStartEndfade           = JSON::ReadFloat2(d, "dayLightStartEndfade");
	skySettings.nightLightStartEndfade         = JSON::ReadFloat2(d, "nightLightStartEndfade");
	skySettings.shadowMapDistance              = JSON::ReadFloat3(d, "shadowMapDistance");
	skySettings.startRotation                  = JSON::ReadFloat3(d, "startRotation");
	skySettings.endRotation                    = JSON::ReadFloat3(d, "endRotation");
	skySettings.normalDirLightColor            = JSON::ReadFloat4(d, "normalDirLightColor");
	skySettings.sunsetDirLightColor            = JSON::ReadFloat4(d, "sunsetDirLightColor");
	skySettings.nightDirLightColor             = JSON::ReadFloat4(d, "nightDirLightColor");
	skySettings.topSkyColorDay                 = JSON::ReadFloat4(d, "topSkyColorDay");
	skySettings.topSkyColorSunSet              = JSON::ReadFloat4(d, "topSkyColorSunSet");
	skySettings.topSkyColorNight               = JSON::ReadFloat4(d, "topSkyColorNight");
	skySettings.midSkyColorDay                 = JSON::ReadFloat4(d, "midSkyColorDay");
	skySettings.midSkyColorSunSet              = JSON::ReadFloat4(d, "midSkyColorSunSet");
	skySettings.midSkyColorNight               = JSON::ReadFloat4(d, "midSkyColorNight");
	skySettings.skyBottomColor                 = JSON::ReadFloat4(d, "skyBottomColor");
	skySettings.RENDER_MODE                    = (SKY_DOME_RENDER_MODE)JSON::ReadInt(d, "RENDER_MODE");
	skySettings.cubeMapName                    = JSON::ReadString(d, "cubeMap");
	sunMoon.sun.dayColorTint                   = JSON::ReadFloat3(d, "sunDayColorTint");
	sunMoon.moon.dayColorTint                  = JSON::ReadFloat3(d, "moonDayColorTint");
	sunMoon.sun.sunsetColorTint                = JSON::ReadFloat3(d, "sunSunsetColorTint");
	sunMoon.moon.sunsetColorTint               = JSON::ReadFloat3(d, "moonSunsetColorTint");
	sunMoon.sun.beginEndFade                   = JSON::ReadFloat2(d, "sunBeginEndfade");
	sunMoon.moon.beginEndFade                  = JSON::ReadFloat2(d, "moonBeginEndfade");
	sunMoon.sun.minMaxDst                      = JSON::ReadFloat2(d, "sunMinMaxDst");
	sunMoon.moon.minMaxDst                     = JSON::ReadFloat2(d, "moonMinMaxDst");
	sunMoon.sun.beginEndDstLerp                = JSON::ReadFloat2(d, "sunBeginEndDstLerp");
	sunMoon.moon.beginEndDstLerp               = JSON::ReadFloat2(d, "moonBeginEndDstLerp");
	sunMoon.sun.beginEndColorBlend             = JSON::ReadFloat2(d, "sunBeginEndColorBlend");
	sunMoon.moon.beginEndColorBlend            = JSON::ReadFloat2(d, "moonBeginEndColorBlend");
}