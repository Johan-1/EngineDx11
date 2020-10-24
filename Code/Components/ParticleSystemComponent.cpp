#include "ParticleSystemComponent.h"
#include "Entity.h"
#include "DXManager.h"
#include "Renderer.h"
#include "TexturePool.h"
#include "Time.h"
#include "CameraManager.h"
#include "TransformComponent.h"
#include <algorithm>
#include <random>
#include <chrono>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <fstream>
#include <string>
#include "DXErrorHandler.h"
#include "Systems.h"

ParticleSystemComponent::ParticleSystemComponent() : IComponent(PARTICLE_COMPONENT),
_hasLifetime(false),
numEmitters(0),
_previousPosition(XMFLOAT3(0,0,0))
{
}

ParticleSystemComponent::~ParticleSystemComponent()
{
	delete[] _particleInstanceData;
	delete[] _particleData;
	delete[] _emitterData;
	delete[] _settings;

	for (int i = 0; i < numEmitters; i++)
	{
		_vertexBuffer[i]->Release();
		_indexBuffer[i]->Release();
		_instanceBuffer[i]->Release();
	}

	delete[] _vertexBuffer;
	delete[] _indexBuffer;
	delete[] _instanceBuffer;

	Systems::renderer->RemoveParticleSystemFromRenderer(this);
}

void ParticleSystemComponent::Init(const char* particleFile)
{
	// get particle settings
	ParsefromJson(particleFile);

	// create arrays of memory
	AllocateMemory();

	// setup based on settings
	SetUp();
}

void ParticleSystemComponent::Init(std::vector<ParticleSettings> settings)
{
	numEmitters = settings.size();
	_settings    = new ParticleSettings[numEmitters];

	// set all settings
	for (int i = 0; i < numEmitters; i++)
		_settings[i] = settings[i];

	// create arrays of memory
	AllocateMemory();

	// setup based on settings
	SetUp();
}

void ParticleSystemComponent::AllocateMemory()
{
	// allocate memory for all settings
	_vertexBuffer         = new ID3D11Buffer*[numEmitters];
	_indexBuffer          = new ID3D11Buffer*[numEmitters];
	_instanceBuffer       = new ID3D11Buffer*[numEmitters];

	_particleInstanceData = new ParticleInstanceType*[numEmitters];
	_particleData         = new ParticleData*[numEmitters];

	_emitterData          = new EmitterData[numEmitters];
	textures              = new ID3D11ShaderResourceView*[numEmitters] {nullptr};

	// allocate arrays for each particle
	for (int i = 0; i < numEmitters; i++)
	{
		_particleInstanceData[i] = new ParticleInstanceType[_settings[i].numParticles];
		_particleData[i]         = new ParticleData[_settings[i].numParticles];
	}
}

void ParticleSystemComponent::SetUp()
{
	// add to renderer 
	Systems::renderer->AddParticleSystemToRenderer(this);

	// get pointer to entity transform
	_transform = GetComponent<TransformComponent>();

	// loop over all emitters and do neccesary setup
	for (int i = 0; i < numEmitters; i++)
	{				
		// if in burst mode we use one timer for every particle per emitter
		_emitterData[i].lifeTime = _settings[i].particleLifetime;

		// apend texture name to directory of the textures
		std::string tp = "Data/Textures/";		
		tp.append(_settings[i].texturePath.c_str());	

		// conert to wide string (only supports asciII characters)
		std::wstring wtp(tp.begin(), tp.end());
		
		// get texture if string is not empty
		// else get defualt white texture
		textures[i] = _settings[i].texturePath.c_str() != "" ? Systems::texturePool->GetTexture(wtp.c_str()) : Systems::texturePool->GetTexture(L"textures/defaultDiffuse.dds");

		// check if emitter have a limited lifetime or not
		// if one emitter has lifetime we will destroy the
		// entire system when lifetime is out
		if (_settings[i].emitterLifetime > 0)
			_hasLifetime = true;

		// create all D3D11 buffers and two triangles for rendering
		CreateBuffers(_settings[i].startSize, i);

		// spawn all particles if this emitter has burst set to true
		if (_settings[i].burst)
			SpawnAllParticles(i);
		else
		{
			// start all particles inactive and set scale to 0 so inactive particles wont be visible
			for (int y = 0; y < _settings[i].numParticles; y++)
			{
				_particleData[i][y].active = false;
				_particleData[i][y].scale = XMFLOAT3(0, 0, 0);
			}

			// get spawnratio based on num particles and lifetime per particle
			_emitterData[i].spawnRatio = _settings[i].particleLifetime / _settings[i].numParticles;
			_emitterData[i].spawnTimer = _emitterData[i].spawnRatio;
		}		
	}
	
	// get current entity position for use next frame
	_previousPosition = _transform->position;
}

void ParticleSystemComponent::CreateBuffers(XMFLOAT2 size, unsigned int index)
{	
	// get device
    ID3D11Device*& device = Systems::dxManager->device;

	// get half size
	float halfX = size.x * 0.5f;
	float halfY = size.y * 0.5f;

	// create vertices
	ParticleVertexType vertices[4]; 
	vertices[0].position  = XMFLOAT3(-halfX, halfY, 0.0f);
	vertices[0].texCoords = XMFLOAT2(0.0f, 0.0f);

	vertices[1].position  = XMFLOAT3(halfX, halfY, 0.0f);
	vertices[1].texCoords = XMFLOAT2(1.0f, 0.0f);

	vertices[2].position  = XMFLOAT3(-halfX, -halfY, 0.0f);
	vertices[2].texCoords = XMFLOAT2(0.0f, 1.0f);

	vertices[3].position  = XMFLOAT3(halfX, -halfY, 0.0f);
	vertices[3].texCoords = XMFLOAT2(1.0f, 1.0f);

	// create indices
	unsigned long indices[6] { 0,1,2,2,1,3 };

	// create the descriptions and resource data to buffers
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc, instanceBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData, instanceData;

	// Set up the description of the vertex buffer.
	vertexBufferDesc.Usage               = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth           = sizeof(ParticleVertexType) * 4;
	vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags      = 0;
	vertexBufferDesc.MiscFlags           = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Set up the description of the index buffer.
	indexBufferDesc.Usage               = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth           = sizeof(unsigned long) * 6;
	indexBufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags      = 0;
	indexBufferDesc.MiscFlags           = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Set up the description of the instance buffer.
	instanceBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth           = sizeof(ParticleInstanceType) * _settings[index].numParticles;
	instanceBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	instanceBufferDesc.MiscFlags           = 0;
	instanceBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem          = vertices;
	vertexData.SysMemPitch      = 0;
	vertexData.SysMemSlicePitch = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem          = indices;
	indexData.SysMemPitch      = 0;
	indexData.SysMemSlicePitch = 0;

	// Give the subresource structure a pointer to the instance data.
	instanceData.pSysMem          = _particleInstanceData[index];
	instanceData.SysMemPitch      = 0;
	instanceData.SysMemSlicePitch = 0;

	HRESULT result;

	// create vertex, index and instance buffers
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer[index]);
	if (FAILED(result))
		DX_ERROR::PrintError(result, "failed to create vertex buffer for particle emitter");

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer[index]);
	if (FAILED(result))
		DX_ERROR::PrintError(result, "failed to create index buffer for particle emitter");

	result = device->CreateBuffer(&instanceBufferDesc, &instanceData, &_instanceBuffer[index]);
	if (FAILED(result))
		DX_ERROR::PrintError(result, "failed to create instance buffer for particle emitter");
}

void ParticleSystemComponent::SpawnAllParticles(unsigned int index)
{
	// loop over and spawn all particles
	for (int i = 0; i < _settings[index].numParticles; i++)	
		SpawnParticle(_particleData[index][i], index);
	
	// set the emitter lifetime to the particle lifetime
	_emitterData[index].lifeTime = _settings[index].particleLifetime;
}

void ParticleSystemComponent::SpawnParticle(ParticleData& particle, unsigned int index)
{
	 // get entity position
	const XMFLOAT3& emitterPos = _transform->position;

	// get min/max spawn points from the position of entity
	XMFLOAT2 spawnMinMaxX = XMFLOAT2(emitterPos.x - (_settings[index].spawnRadius * 0.5f), emitterPos.x + (_settings[index].spawnRadius * 0.5f));
	XMFLOAT2 spawnMinMaxY = XMFLOAT2(emitterPos.y - (_settings[index].spawnRadius * 0.5f), emitterPos.y + (_settings[index].spawnRadius * 0.5f));
	XMFLOAT2 spawnMinMaxZ = XMFLOAT2(emitterPos.z - (_settings[index].spawnRadius * 0.5f), emitterPos.z + (_settings[index].spawnRadius * 0.5f));

	// get min/max initial direction values
	XMFLOAT2 directionMinMaxX = XMFLOAT2(_settings[index].direction.x - (_settings[index].velocitySpread.x * 0.5f), _settings[index].direction.x + (_settings[index].velocitySpread.x * 0.5f));
	XMFLOAT2 directionMinMaxY = XMFLOAT2(_settings[index].direction.y - (_settings[index].velocitySpread.y * 0.5f), _settings[index].direction.y + (_settings[index].velocitySpread.y * 0.5f));
	XMFLOAT2 directionMinMaxZ = XMFLOAT2(_settings[index].direction.z - (_settings[index].velocitySpread.z * 0.5f), _settings[index].direction.z + (_settings[index].velocitySpread.z * 0.5f));

	// get random velocity direction from min7max values
	XMFLOAT3 direction = XMFLOAT3(GetRandomFloat(directionMinMaxX.x, directionMinMaxX.y), 
		                          GetRandomFloat(directionMinMaxY.x, directionMinMaxY.y),
		                          GetRandomFloat(directionMinMaxZ.x, directionMinMaxZ.y));

	// normalize the result
	XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));

	// if we simulate in local space transform direction from worldspace
	if (_settings[index].LocalSpace)
		direction = GetDirectionLocal(direction);

	// get random color values
	XMFLOAT3 startColorMultiplier = XMFLOAT3
									(GetRandomFloat(_settings[index].startColorMultiplierRGBMin.x, _settings[index].startColorMultiplierRGBMax.x), 
									 GetRandomFloat(_settings[index].startColorMultiplierRGBMin.y, _settings[index].startColorMultiplierRGBMax.y),
									 GetRandomFloat(_settings[index].startColorMultiplierRGBMin.z, _settings[index].startColorMultiplierRGBMax.z));

	XMFLOAT3 endColorMultiplier = XMFLOAT3
									(GetRandomFloat(_settings[index].endColorMultiplierRGBMin.x, _settings[index].endColorMultiplierRGBMax.x),
									 GetRandomFloat(_settings[index].endColorMultiplierRGBMin.y, _settings[index].endColorMultiplierRGBMax.y),
									 GetRandomFloat(_settings[index].endColorMultiplierRGBMin.z, _settings[index].endColorMultiplierRGBMax.z));

	// get random speed
	float speed = GetRandomFloat(_settings[index].minMaxSpeed.x, _settings[index].minMaxSpeed.y);

	float startScale = GetRandomFloat(_settings[index].startScaleMinMax.x, _settings[index].startScaleMinMax.y);
	float endScale   = GetRandomFloat(_settings[index].endScaleMinMax.x,   _settings[index].endScaleMinMax.y);

	// get rotation speed from min7max values
	float rotationSpeed = GetRandomFloat(_settings[index].rotationPerSecMinMax.x, _settings[index].rotationPerSecMinMax.y);

	// get spawn position
	XMStoreFloat3(&particle.position,
		XMVectorAdd(XMLoadFloat3(&XMFLOAT3(GetRandomFloat(spawnMinMaxX.x, spawnMinMaxX.y), 
			                               GetRandomFloat(spawnMinMaxY.x, spawnMinMaxY.y), 
			                               GetRandomFloat(spawnMinMaxZ.x, spawnMinMaxZ.y))),
			        XMLoadFloat3(&_settings[index].spawnOffset)));

	// set start values
	particle.zRotation               = 0;
	particle.scale                   = XMFLOAT3(1, 1, 1);
	particle.direction               = direction;
	particle.speed.x                 = speed;
	particle.speed.y                 = speed;
	particle.speed.z                 = speed;
	particle.dragMultiplier          = 1.0f;
	particle.lifeTime                = _settings[index].particleLifetime;
	particle.active                  = true;
	particle.currentColorMultiplier  = _settings[index].startColorMultiplierRGBMin;
	particle.currentAlpha            = _settings[index].startAlpha;
	particle.fraction                = 0;
	particle.startColorMultiplierRGB = startColorMultiplier;
	particle.endColorMultiplierRGB   = endColorMultiplier;
	particle.startScale              = startScale;
	particle.endScale                = endScale;
	particle.zRotationSpeed          = rotationSpeed;
}

void ParticleSystemComponent::Update(const float& delta)
{
	// update simulation
	UpdateLifeTime(delta);
	UpdateVelocity(delta);
	UpdateLerps(delta);
	UpdateRotations(delta);

	// sort particles if the emitter uses alpha blend
	for(int i =0; i< numEmitters; i++)
		if(_settings[i].BLEND == BLEND_STATE::BLEND_ALPHA)
		   SortParticles(i);

	// update buffers
	UpdateBuffer();		
}

void ParticleSystemComponent::UpdateVelocity(const float& delta)
{
	XMFLOAT3 EmitterPos = _transform->position;

	// get velocity rom last pos to currentpos
	XMFLOAT3 velocityVector;
	XMStoreFloat3(&velocityVector, XMVectorSubtract(XMLoadFloat3(&EmitterPos), XMLoadFloat3(&_previousPosition)));

	for (int i = 0; i < numEmitters; i++)
	{
		for (int y =0; y < _settings[i].numParticles; y++)
		{
			// if the particle is not active skip it
			if (!_particleData[i][y].active)
				continue;

			// save last frame position before we update velocity (used to calculate velocity direction of particle for rotation) 
			_particleData[i][y].previousPosition = _particleData[i][y].position;

			// manipulate speed based on gravity and direction
			// if direction is negative we have to take -gravity resulting in the speed decreasing with a positive gravity and increasing with a negative gravity (becuase of doubble negative)
			// positive direction works normally, will add negative gravity resulting in speed slowing down or add positive gravity that will increase speed
			if (_particleData[i][y].direction.x < 0.0f)
				_particleData[i][y].speed.x -= _settings[i].gravity.x * delta;
			else
				_particleData[i][y].speed.x += _settings[i].gravity.x * delta;

			if (_particleData[i][y].direction.y < 0.0f)
				_particleData[i][y].speed.y -= _settings[i].gravity.y * delta;
			else
				_particleData[i][y].speed.y += _settings[i].gravity.y * delta;

			if (_particleData[i][y].direction.z < 0.0f)
				_particleData[i][y].speed.z -= _settings[i].gravity.z * delta;
			else
				_particleData[i][y].speed.z += _settings[i].gravity.z * delta;

			// update the drag
			_particleData[i][y].dragMultiplier -= _settings[i].drag * delta;
			if (_particleData[i][y].dragMultiplier < 0.0f)
				_particleData[i][y].dragMultiplier = 0;

			// update the position based on direction and speed
			_particleData[i][y].position.x += (_particleData[i][y].direction.x * _particleData[i][y].speed.x * _particleData[i][y].dragMultiplier * delta) + velocityVector.x * _settings[i].inheritVelocityScale.x;
			_particleData[i][y].position.y += (_particleData[i][y].direction.y * _particleData[i][y].speed.y * _particleData[i][y].dragMultiplier * delta) + velocityVector.y * _settings[i].inheritVelocityScale.y;
			_particleData[i][y].position.z += (_particleData[i][y].direction.z * _particleData[i][y].speed.z * _particleData[i][y].dragMultiplier * delta) + velocityVector.z * _settings[i].inheritVelocityScale.z;

			// add on gravity
			_particleData[i][y].position.x += _settings[i].gravity.x * _particleData[i][y].dragMultiplier * delta;
			_particleData[i][y].position.y += _settings[i].gravity.y * _particleData[i][y].dragMultiplier * delta;
			_particleData[i][y].position.z += _settings[i].gravity.z * _particleData[i][y].dragMultiplier * delta;

			// add on the movement of entity from last to current frame 
			// if the particles should follower the enity transform
			if (_settings[i].followEmitter)
				XMStoreFloat3(&_particleData[i][y].position, XMVectorAdd(XMLoadFloat3(&_particleData[i][y].position), XMLoadFloat3(&velocityVector)));				
		}		
	}
	// save position for next frame
	_previousPosition = EmitterPos;
}

void ParticleSystemComponent::UpdateLerps(const float& delta)
{
	for (int i = 0; i < numEmitters; i++)
	{
		for (int y =0; y < _settings[i].numParticles; y++)
		{
			if (!_particleData[i][y].active)
				continue;

			// add on to fraction
			_particleData[i][y].fraction += delta / _settings[i].particleLifetime;

			// get color multiplier
			XMStoreFloat3(&_particleData[i][y].currentColorMultiplier, 
				XMVectorLerp(XMLoadFloat3(&_particleData[i][y].startColorMultiplierRGB), 
					         XMLoadFloat3(&_particleData[i][y].endColorMultiplierRGB), _particleData[i][y].fraction));

			// lerp alpha value, this work as transparancy for additive particles aswell
			_particleData[i][y].currentAlpha = LerpFloat(_settings[i].startAlpha, _settings[i].endAlpha, _particleData[i][y].fraction);

			// get scale
			float currentScale = LerpFloat(_particleData[i][y].startScale, _particleData[i][y].endScale, _particleData[i][y].fraction);
			_particleData[i][y].scale = XMFLOAT3(currentScale, currentScale, currentScale);			
		}		
	}
}

void ParticleSystemComponent::UpdateLifeTime(const float& delta)
{
	for (int i =0; i < numEmitters; i++)
	{
		// if has a lifetime, countdown and remove entity
		if (_hasLifetime)
		{
			_settings[i].emitterLifetime -= delta;
			if (_settings[i].emitterLifetime <= 0)
				parent->RemoveEntity();
		}

		// if in burst mode only update the timer that all particles share
		if (_settings[i].burst)
		{
			_emitterData[i].lifeTime -= delta;
			if (_emitterData[i].lifeTime <= 0)
				SpawnAllParticles(i);
			continue;
		}

		// if flow mode update the lifespan of all active particles
		for (int y = 0; y < _settings[i].numParticles; y++)
		{
			if (_particleData[i][y].active)
			{
				_particleData[i][y].lifeTime -= delta;
				if (_particleData[i][y].lifeTime <= 0)
					SpawnParticle(_particleData[i][y], i);
			}
		}

		// check if all particles is alredy active and spawned
		if (_emitterData[i].numSpawnedParticles == _settings[i].numParticles)
			continue;

		// spawn new particle based on number of particles in emitter and lifespan of 1 particle to get a nice flow
		_emitterData[i].spawnTimer += delta;
		if (_emitterData[i].spawnTimer >= _emitterData[i].spawnRatio)
		{
			_emitterData[i].spawnTimer = 0;
			SpawnParticle(_particleData[i][_emitterData[i].numSpawnedParticles],i);
			_emitterData[i].numSpawnedParticles++;
		}
	}	
}

void ParticleSystemComponent::SortParticles(unsigned int index)
{
	XMFLOAT3 camPos = Systems::cameraManager->currentCameraGame->GetComponent<TransformComponent>()->position;
	XMFLOAT3 vec;

	// get distance of particle from camera
	for (int i = 0; i < _settings[index].numParticles; i++)
	{
		XMStoreFloat3(&vec, XMVectorSubtract(XMLoadFloat3(&_particleData[index][i].position), XMLoadFloat3(&camPos)));
		XMStoreFloat(&_particleData[index][i].distance, XMVector3Length(XMLoadFloat3(&vec)));
	}
			
	std::sort(_particleData[index], _particleData[index] + _settings[index].numParticles, [&](ParticleData a, ParticleData b) -> bool {return a.distance > b.distance; });
}

float ParticleSystemComponent::GetRandomFloat(float min, float max)
{
	unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 generator(seed);
	std::uniform_real_distribution<float> distribution(min, max);

	return distribution(generator);
}

float ParticleSystemComponent::LerpFloat(float a, float b, float f)
{
	return (a * (1.0f - f)) + (b * f);
}

XMFLOAT3 ParticleSystemComponent::GetDirectionLocal(XMFLOAT3 direction)
{
	const XMFLOAT3& emitterRotation = _transform->rotation;
	XMFLOAT4X4 matrixRotation; XMStoreFloat4x4(&matrixRotation, XMMatrixIdentity());

	XMStoreFloat4x4(&matrixRotation, XMMatrixRotationRollPitchYaw(XMConvertToRadians(emitterRotation.x), XMConvertToRadians(emitterRotation.y), XMConvertToRadians(emitterRotation.z)));
	XMStoreFloat3(&direction, XMVector3TransformCoord(XMLoadFloat3(&direction), XMLoadFloat4x4(&matrixRotation)));
	XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));
	
	return direction;	
}

void ParticleSystemComponent::UpdateRotations(const float& delta)
{
	XMFLOAT3 forward, up, right;
	Systems::cameraManager->currentCameraGame->GetComponent<TransformComponent>()->GetAllAxis(forward, right, up);

	XMFLOAT4X4 rotationMatrix; XMStoreFloat4x4(&rotationMatrix, XMMatrixIdentity());

	 for(int i =0; i < numEmitters; i++)
	 {
		 if (_settings[i].rotationByVelocity) // billboarding with z rotation by velocityvector
		 {
			 for (int y = 0; y < _settings[i].numParticles; y++)
			 {
				 // get the direction of velocity
				 XMFLOAT3 dir; XMStoreFloat3(&dir, XMVectorSubtract(XMLoadFloat3(&_particleData[i][y].position), XMLoadFloat3(&_particleData[i][y].previousPosition))); 
				 XMStoreFloat3(&dir, XMVector3Normalize(XMLoadFloat3(&dir)));

				 // get perpendicular vector from camera forward and inverted velocity vector
				 XMFLOAT3 byVelocityRight(0, 0, 0);
				 XMStoreFloat3(&byVelocityRight, XMVector3Cross(XMLoadFloat3(&forward), XMVectorNegate(XMLoadFloat3(&dir))));
				 XMStoreFloat3(&byVelocityRight, XMVector3Normalize(XMLoadFloat3(&byVelocityRight)));				

				 // get the perpendicular vector from camera forward and the newly calculated rightvector 
				 XMFLOAT3 byVelocityUp(0, 0, 0);
				 XMStoreFloat3(&byVelocityUp, XMVector3Cross(XMLoadFloat3(&forward), XMLoadFloat3(&byVelocityRight)));
				 XMStoreFloat3(&byVelocityUp, XMVector3Normalize(XMLoadFloat3(&byVelocityUp)));

				 rotationMatrix._11 = byVelocityRight.x;
				 rotationMatrix._21 = byVelocityRight.y;
				 rotationMatrix._31 = byVelocityRight.z;

				 rotationMatrix._12 = byVelocityUp.x;
				 rotationMatrix._22 = byVelocityUp.y;
				 rotationMatrix._32 = byVelocityUp.z;

				 rotationMatrix._13 = forward.x;
				 rotationMatrix._23 = forward.y;
				 rotationMatrix._33 = forward.z;

				 XMStoreFloat4x4(&_particleData[i][y].rotationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&rotationMatrix)));
			 }
		 }
		 else // billboarding with z rotation by speed
		 {
			 rotationMatrix._11 = right.x;
			 rotationMatrix._21 = right.y;
			 rotationMatrix._31 = right.z;

			 rotationMatrix._12 = up.x;
			 rotationMatrix._22 = up.y;
			 rotationMatrix._32 = up.z;

			 rotationMatrix._13 = forward.x;
			 rotationMatrix._23 = forward.y;
			 rotationMatrix._33 = forward.z;

			 XMFLOAT4X4 zRotationMatrix;  XMStoreFloat4x4(&zRotationMatrix, XMMatrixIdentity());

			 for (int y = 0; y < _settings[i].numParticles; y++)
			 {
				 // add on rotation for each particle and caculate the rotationmatrix for the z rotation
				 _particleData[i][y].zRotation += _particleData[i][y].zRotationSpeed * delta;

				 XMStoreFloat4x4(&zRotationMatrix, XMMatrixRotationZ(XMConvertToRadians(_particleData[i][y].zRotation)));
				 XMStoreFloat4x4(&_particleData[i][y].rotationMatrix, XMMatrixMultiplyTranspose(XMLoadFloat4x4(&rotationMatrix), XMLoadFloat4x4(&zRotationMatrix)));
			 }
		 }
	 }					
}

void ParticleSystemComponent::UpdateBuffer() 
{
	XMFLOAT4X4 matrixPosition; XMStoreFloat4x4(&matrixPosition, XMMatrixIdentity());
	XMFLOAT4X4 matrixScale;    XMStoreFloat4x4(&matrixScale,    XMMatrixIdentity());
	
	XMFLOAT3 color;
	float alpha;
			
	for (int i =0; i < numEmitters; i++)
	{
		for (int y = 0; y < _settings[i].numParticles; y++)
		{
			// calculate all matrices from the sorted particle values
			XMStoreFloat4x4(&matrixPosition, XMMatrixTranslationFromVector(XMLoadFloat3(&_particleData[i][y].position)));
			XMStoreFloat4x4(&matrixScale,    XMMatrixScalingFromVector(XMLoadFloat3(&_particleData[i][y].scale)));

			// calculate worldmatrix
			XMStoreFloat4x4(&_particleInstanceData[i][y].worldMatrix, XMMatrixMultiply(XMLoadFloat4x4(&matrixScale), XMLoadFloat4x4(&_particleData[i][y].rotationMatrix)));
			XMStoreFloat4x4(&_particleInstanceData[i][y].worldMatrix, XMMatrixMultiply(XMLoadFloat4x4(&_particleInstanceData[i][y].worldMatrix), XMLoadFloat4x4(&matrixPosition)));

			color = _particleData[i][y].currentColorMultiplier;
			alpha = _particleData[i][y].currentAlpha;

			// multiply all colors with alpha value, black == transparent
			if (_settings[i].BLEND == BLEND_STATE::BLEND_ADDITIVE || _settings[i].BLEND == BLEND_STATE::BLEND_SUBTRACTIVE)
			{
				XMStoreFloat4(&_particleInstanceData[i][y].color,
					XMVectorMultiply(XMLoadFloat4(&XMFLOAT4(color.x, color.y, color.z, 1)), XMLoadFloat4(&XMFLOAT4(alpha, alpha, alpha, alpha))));
			}
			else if (_settings[i].BLEND == BLEND_STATE::BLEND_ALPHA)
				XMStoreFloat4(&_particleInstanceData[i][y].color, XMLoadFloat4(&XMFLOAT4(color.x, color.y, color.z, alpha)));
		}

		ID3D11DeviceContext*& devCon = Systems::dxManager->devCon;
		D3D11_MAPPED_SUBRESOURCE data;

		// map instancebuffer
		HRESULT result = devCon->Map(_instanceBuffer[i], 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to map instance buffer for particle emitter");

		// copy the instancedata over to the instancebuffer
		memcpy(data.pData, (void*)_particleInstanceData[i], sizeof(ParticleInstanceType) * _settings[i].numParticles);

		//unmap
		devCon->Unmap(_instanceBuffer[i], 0);
	}	
}

void ParticleSystemComponent::UploadBuffers(unsigned int index)
{
	ID3D11DeviceContext*& devCon = Systems::dxManager->devCon;

	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferptrs[2];

	// Set vertex buffer stride and offset.
	strides[0] = sizeof(ParticleVertexType);
	strides[1] = sizeof(ParticleInstanceType);

	offsets[0] = 0;
	offsets[1] = 0;

	bufferptrs[0] = _vertexBuffer[index];
	bufferptrs[1] = _instanceBuffer[index];

	// Set the vertex buffer and instance buffer
	devCon->IASetVertexBuffers(0, 2, bufferptrs, strides, offsets);

	// Set the index buffer 
	devCon->IASetIndexBuffer(_indexBuffer[index], DXGI_FORMAT_R32_UINT, 0);
}

void ParticleSystemComponent::ParsefromJson(const char* file)
{
	FILE* fp; fopen_s(&fp, file, "rb");
	char readBuffer[65536];
	rapidjson::FileReadStream inStream(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document d;
	d.ParseStream(inStream);
	fclose(fp);

	assert(d.IsObject());

	assert(d["numEmitters"].IsInt());
	numEmitters = d["numEmitters"].GetInt();

	_settings = new ParticleSettings[numEmitters];
	
	std::string key = "";

	for(int i =0; i < numEmitters; i++)
	{
		std::string index = std::to_string(i);

		key = "numParticles";
		key.append(index.c_str());
		assert(d[key.c_str()].IsInt());
		_settings[i].numParticles = d[key.c_str()].GetInt();

		key = "texture";
		key.append(index.c_str());
		assert(d[key.c_str()].IsString());
		_settings[i].texturePath = std::string(d[key.c_str()].GetString(), 100);

		key = "startSize";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		rapidjson::Value& a = d[key.c_str()];
		_settings[i].startSize = XMFLOAT2(a[0].GetFloat(), a[1].GetFloat());

		key = "direction";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].direction = XMFLOAT3(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());

		key = "minMaxSpeed";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].minMaxSpeed = XMFLOAT2(a[0].GetFloat(), a[1].GetFloat());

		key = "gravity";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].gravity = XMFLOAT3(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());

		key = "drag";
		key.append(index.c_str());
		assert(d[key.c_str()].IsFloat());
		_settings[i].drag = d[key.c_str()].GetFloat();

		key = "velocitySpread";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].velocitySpread = XMFLOAT3(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());

		key = "emitterLifetime";
		key.append(index.c_str());
		assert(d[key.c_str()].IsFloat());
		_settings[i].emitterLifetime = d[key.c_str()].GetFloat();

		key = "particleLifetime";
		key.append(index.c_str());
		assert(d[key.c_str()].IsFloat());
		_settings[i].particleLifetime = d[key.c_str()].GetFloat();

		key = "spawnRadius";
		key.append(index.c_str());
		assert(d[key.c_str()].IsFloat());
		_settings[i].spawnRadius = d[key.c_str()].GetFloat();

		key = "burst";
		key.append(index.c_str());
		assert(d[key.c_str()].IsBool());
		_settings[i].burst = d[key.c_str()].GetBool();

		key = "followEmitter";
		key.append(index.c_str());
		assert(d[key.c_str()].IsBool());
		_settings[i].followEmitter = d[key.c_str()].GetBool();

		key = "startColorMultiplierRGBMin";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].startColorMultiplierRGBMin = XMFLOAT3(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());

		key = "startColorMultiplierRGBMax";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].startColorMultiplierRGBMax = XMFLOAT3(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());

		key = "endColorMultiplierRGBMin";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].endColorMultiplierRGBMin = XMFLOAT3(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());

		key = "endColorMultiplierRGBMax";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].endColorMultiplierRGBMax = XMFLOAT3(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());

		key = "startAlpha";
		key.append(index.c_str());
		assert(d[key.c_str()].IsFloat());
		_settings[i].startAlpha = d[key.c_str()].GetFloat();

		key = "endAlpha";
		key.append(index.c_str());
		assert(d[key.c_str()].IsFloat());
		_settings[i].endAlpha = d[key.c_str()].GetFloat();

		key = "BLEND";
		key.append(index.c_str());
		assert(d[key.c_str()].IsInt());
		_settings[i].BLEND = (BLEND_STATE)d[key.c_str()].GetInt();

		key = "spawnOffset";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].spawnOffset = XMFLOAT3(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());

		key = "localSpace";
		key.append(index.c_str());
		assert(d[key.c_str()].IsBool());
		_settings[i].LocalSpace = d[key.c_str()].GetBool();

		key = "startScaleMinMax";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].startScaleMinMax = XMFLOAT2(a[0].GetFloat(), a[1].GetFloat());

		key = "endScaleMinMax";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].endScaleMinMax = XMFLOAT2(a[0].GetFloat(), a[1].GetFloat());

		key = "rotationByVelocity";
		key.append(index.c_str());
		assert(d[key.c_str()].IsBool());
		_settings[i].rotationByVelocity = d[key.c_str()].GetBool();

		key = "rotationPerSecMinMax";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].rotationPerSecMinMax = XMFLOAT2(a[0].GetFloat(), a[1].GetFloat());

		key = "inheritVelocityScale";
		key.append(index.c_str());
		assert(d[key.c_str()].IsArray());
		a = d[key.c_str()];
		_settings[i].inheritVelocityScale = XMFLOAT3(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());
	}			
}