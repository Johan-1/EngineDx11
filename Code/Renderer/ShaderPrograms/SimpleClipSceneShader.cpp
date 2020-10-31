#include "SimpleClipSceneShader.h"
#include "ShaderHelpers.h"
#include "DXManager.h"
#include "DXBlendStates.h"
#include "DXDepthStencilStates.h"
#include "CameraManager.h"
#include "LightManager.h"
#include "Mesh.h"
#include "Systems.h"
#include "SystemDefs.h"
#include "Entity.h"
#include "QuadComponent.h"
#include "ParticleShader.h"
#include "SkyDome.h"
#include "MathHelpers.h"
#include "InstancedModel.h"

SimpleClipSceneShader::SimpleClipSceneShader(const unsigned int resX, const unsigned int resY)
{
	// create shaders
	SHADER_HELPERS::CreateVertexShader(L"Data/shaders/vertexSimpleClip.shader", _vertexShader, vertexShaderByteCode);
	SHADER_HELPERS::CreateVertexShader(L"Data/shaders/vertexSimpleClipInstanced.shader", _vertexShaderInstanced, vertexShaderByteCodeInstanced);
	SHADER_HELPERS::CreatePixelShader(L"Data/shaders/pixelSimpleClip.shader",   _pixelShader,  pixelShaderByteCode);

	// create constant buffers
	SHADER_HELPERS::CreateConstantBuffer(_CBVertex);
	SHADER_HELPERS::CreateConstantBuffer(_CBPixel);
	SHADER_HELPERS::CreateConstantBuffer(_CBVertexInstanced);

	// create render texture
	_renderTexture = new RenderToTexture(resX, resY, false, SystemSettings::USE_HDR, false);
}

SimpleClipSceneShader::~SimpleClipSceneShader()
{
	_vertexShader->Release();
	_vertexShaderInstanced->Release();
	_pixelShader->Release();

	vertexShaderByteCode->Release();
	vertexShaderByteCodeInstanced->Release();
	pixelShaderByteCode->Release();

	_CBVertex->Release();
}

void SimpleClipSceneShader::RenderScene(std::vector<Mesh*>& opaqueMeshes, std::vector<InstancedModel*>& instancedModels, XMFLOAT4 clipPlane, bool includeSkyBox, bool includeParticles)
{
	// get systems
	DXManager&     DXM = *Systems::dxManager;
	CameraManager& CM  = *Systems::cameraManager;
	LightManager&  LM  = *Systems::lightManager;

	// get device context
	ID3D11DeviceContext*& devCon = DXM.devCon;

	// get game camera and shadow camera
	CameraComponent*& camera = CM.currentCameraGame;

	// set constant data that all meshes share
	CBVertex constantVertex;
	XMStoreFloat4(&constantVertex.clipingPlane, XMLoadFloat4(&clipPlane));

	CBPixel  constantPixel;
	const XMFLOAT3& camPos  = camera->GetComponent<TransformComponent>()->position;
	constantPixel.cameraPos = XMFLOAT4(camPos.x, camPos.y, camPos.z, 1.0f);
	
	// clear our reflection map render texture and set it to active
	_renderTexture->ClearRenderTarget(0, 0, 0, 1, false);
	_renderTexture->SetRendertarget(false, false);

	// render skybox first
	if (includeSkyBox)
		Systems::renderer->skyDome->Render(true);

	// set opaque blend state
	DXM.blendStates->SetBlendState(BLEND_STATE::BLEND_OPAQUE);

	// set shaders that will handle rednering opaque meshes to reflectionmap
	devCon->VSSetShader(_vertexShader, NULL, 0);
	devCon->PSSetShader(_pixelShader, NULL, 0);

	// set constant buffer for the vertex and pixel shader
	devCon->VSSetConstantBuffers(0, 1, &_CBVertex);
	devCon->PSSetConstantBuffers(0, 1, &LM.cbAmbDir);
	devCon->PSSetConstantBuffers(1, 1, &LM.cbPoint);
	devCon->PSSetConstantBuffers(2, 1, &_CBPixel);

	//-------------------------------------------------------------------------------------------- OPAQUE
	// loop over all opaque meshes that is set to cast reflections
	size_t size = opaqueMeshes.size();
	for (int y = 0; y < size; y++)
	{
		Mesh*& mesh = opaqueMeshes[y];

		// get world matrix of mesh and update the buffer
		XMStoreFloat4x4(&constantVertex.world, XMLoadFloat4x4(&mesh->GetWorldMatrixTrans()));
		XMStoreFloat4x4(&constantVertex.worldViewProj, XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&mesh->GetWorldMatrix(), &camera->viewProjMatrix)));

		// set pixel constant data
		constantPixel.hasHeightmap = mesh->hasHeightmap;
		constantPixel.heightScale  = mesh->heightMapScale;

		SHADER_HELPERS::UpdateConstantBuffer((void*)&constantVertex, sizeof(CBVertex), _CBVertex);
		SHADER_HELPERS::UpdateConstantBuffer((void*)&constantPixel,  sizeof(CBPixel),  _CBPixel);

		// get the texture array of mesh
		ID3D11ShaderResourceView** meshTextures = mesh->baseTextures;

		// specular map is not sent to this shader tho this is not calculated
		ID3D11ShaderResourceView* texArray[3] = { meshTextures[0], meshTextures[1], meshTextures[3] };
		devCon->PSSetShaderResources(0, 3, texArray);

		// upload and draw the mesh
		mesh->UploadBuffers();

		devCon->DrawIndexed(mesh->numIndices, 0, 0);
	}

	// unbind so we can use resources as input in next stages
	ID3D11ShaderResourceView* nullSRV[3] = { NULL, NULL, NULL };
	devCon->PSSetShaderResources(0, 3, nullSRV);

	//-------------------------------------------------------------------------------------------- INSTANCED OPAQUE
	// change to instanced shader and constant buffer
	devCon->VSSetShader(_vertexShaderInstanced, NULL, 0);
	devCon->VSSetConstantBuffers(0, 1, &_CBVertexInstanced);
	Systems::renderer->inputLayouts->SetInputLayout(INPUT_LAYOUT_TYPE::LAYOUT_3D_INSTANCED);

	// uppdate constant buffer
	CBVertexInstanced vertexInstanced;
	XMStoreFloat4x4(&vertexInstanced.viewProj, XMLoadFloat4x4(&camera->viewProjMatrixTrans));
	XMStoreFloat4(&vertexInstanced.clipingPlane, XMLoadFloat4(&clipPlane));
	SHADER_HELPERS::UpdateConstantBuffer((void*)&vertexInstanced, sizeof(CBVertexInstanced), _CBVertexInstanced);

	size = instancedModels.size();
	for (int i = 0; i < size; i++)
	{
		InstancedModel*& model = instancedModels[i];
		model->UploadInstances();

		std::vector<Mesh*>& meshes = model->meshes;
		size_t meshesSize = meshes.size();
		for (int y = 0; y < meshesSize; y++)
		{
			Mesh*& mesh = meshes[y];

			// set pixel constant data
			constantPixel.hasHeightmap = mesh->hasHeightmap;
			constantPixel.heightScale  = mesh->heightMapScale;
			SHADER_HELPERS::UpdateConstantBuffer((void*)&constantPixel, sizeof(CBPixel), _CBPixel);

			ID3D11Buffer* vertexBuffer = mesh->vertexBuffer;

			// Set the vertex buffer in slot 0
			devCon->IASetVertexBuffers(0, 1, &vertexBuffer, model->strides, model->offsets);

			// Set the index buffer 
			devCon->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			// specular map is not sent to this shader tho this is not calculated
			ID3D11ShaderResourceView* texArray[3] = { mesh->baseTextures[0], mesh->baseTextures[1], mesh->baseTextures[3] };
			devCon->PSSetShaderResources(0, 3, texArray);

			// draw all instances of this mesh
			devCon->DrawIndexedInstanced(mesh->numIndices, model->numInstances, 0, 0, 0);
		}
	}

	//-------------------------------------------------------------------------------------------- PARTICLES
	// render all particles to the texture 
	if (includeParticles)
	{
		Renderer& renderer = *Systems::renderer;
		std::vector<ParticleSystemComponent*>& particles = renderer.GetParticles();

		if (particles.size() > 0)
		{
			renderer.inputLayouts->SetInputLayout(INPUT_LAYOUT_TYPE::LAYOUT_PARTICLE);
			renderer.particleShader->RenderParticles(particles);
			renderer.inputLayouts->SetInputLayout(INPUT_LAYOUT_TYPE::LAYOUT_3D);
		}
	}
}
