#include "ParticleShader.h"
#include "DXManager.h"
#include "DXDepthStencilStates.h"
#include "CameraComponent.h"
#include "CameraManager.h"
#include "ParticleSystemComponent.h"
#include "ShaderHelpers.h"
#include "Systems.h"

ParticleShader::ParticleShader()
{
	// create and compile shaders
	SHADER_HELPERS::CreateVertexShader(L"Data/shaders/vertexParticle.shader", _vertexShader, vertexShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"Data/shaders/pixelParticle.shader",   _pixelShader,  pixelShaderByteCode);

	// create constant buffer for vertex shader
	SHADER_HELPERS::CreateConstantBuffer(_constantBufferVertex);
}

ParticleShader::~ParticleShader()
{
	vertexShaderByteCode->Release();
	pixelShaderByteCode->Release();

	_vertexShader->Release();
	_pixelShader->Release();

	_constantBufferVertex->Release();
}

void ParticleShader::RenderParticles(std::vector<ParticleSystemComponent*>& systems)
{
	if (systems.size() == 0)
		return;

	// get DX and camera manager
	DXManager& DXM    = *Systems::dxManager;
	CameraManager& CM = *Systems::cameraManager;

	// get device context and camera
	ID3D11DeviceContext*& devCon = DXM.devCon;
	CameraComponent* camera      = CM.currentCameraGame;

	// set shaders			
	devCon->VSSetShader(_vertexShader, NULL, 0);
	devCon->PSSetShader(_pixelShader, NULL, 0);

	// set constant buffer
	devCon->VSSetConstantBuffers(0, 1, &_constantBufferVertex);

	// constantbuffer structures
	CBVertex vertexData;

	// set the vertex constant data
	XMStoreFloat4x4(&vertexData.viewProj, XMLoadFloat4x4(&camera->viewProjMatrixTrans));
	
	// update the vertex constant buffer
	SHADER_HELPERS::UpdateConstantBuffer((void*)&vertexData, sizeof(CBVertex), _constantBufferVertex);

	// don't write the particles to the depth buffer, only read
	DXM.depthStencilStates->SetDepthStencilState(DEPTH_STENCIL_STATE::READ_ONLY);

	// loop over all particle systems
	size_t numSystems = systems.size();
	for (int i = 0; i < numSystems; i++)
	{
		ParticleSystemComponent*& system = systems[i];

		// loop over all emitters in each system
		size_t numEmitters = system->numEmitters;
		for (int y = 0; y < numEmitters; y++)
		{
			// upload vertex and indexbuffers
			system->UploadBuffers(y);

			// set the blend state that this emitter uses
			// TODO: this is probably going to set the same blend state
			// multiple times in a row, fix this
			DXM.blendStates->SetBlendState(system->GetBlendState(y));

			// get texture
			ID3D11ShaderResourceView* texture = system->GetTexture(y);

			// set texture
			devCon->PSSetShaderResources(0, 1, &texture);

			// draw	instanced with the same 6 indices and 4 vertices for all particles	
			devCon->DrawIndexedInstanced(6, system->GetNumParticles(y), 0, 0, 0);
		}
	}
	// unbind so we can use resources as input in next stages
	ID3D11ShaderResourceView* nullSRV[1] = { NULL};
	devCon->PSSetShaderResources(0, 1, nullSRV);

	// enable depth writing
	DXM.depthStencilStates->SetDepthStencilState(DEPTH_STENCIL_STATE::ENABLED);
}
