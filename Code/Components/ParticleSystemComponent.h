#pragma once
#include <DirectXMath.h>
#include "IComponent.h"
#include "DXManager.h"
#include "DXBlendStates.h"
#include "TransformComponent.h"

using namespace DirectX;

class Entity;

// structure for the per particle instance buffer
struct ParticleInstanceType
{
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4   color;
};

// vertex structure
struct ParticleVertexType
{
	XMFLOAT3 position;
	XMFLOAT2 texCoords;
};

// particle data per particle
struct ParticleData
{
	XMFLOAT3   position;
	float      zRotation;
	XMFLOAT3   scale;
	XMFLOAT3   direction;
	XMFLOAT3   speed;
	XMFLOAT3   startColorMultiplierRGB;
	XMFLOAT3   endColorMultiplierRGB;
	XMFLOAT3   currentColorMultiplier;
	float      currentAlpha;
	float      fraction;
	float      lifeTime;
	float      dragMultiplier;
	bool       active;
	float      distance;
	float      startScale;
	float      endScale;
	XMFLOAT3   previousPosition;
	XMFLOAT4X4 rotationMatrix;
	float      zRotationSpeed;
};

struct EmitterData
{
	float        lifeTime;
	float        spawnRatio;
	float        spawnTimer;
	unsigned int numSpawnedParticles;

	EmitterData() :
		lifeTime(0.0f),
		spawnRatio(0.0f),
		spawnTimer(0.0f),
		numSpawnedParticles(0) {};
};

// per emitter settings struct
// will be filled from a .json file
struct ParticleSettings
{
	unsigned int numParticles; 
	std::string  texturePath; 
	XMFLOAT2     startSize;   
	XMFLOAT3     direction; 
	XMFLOAT2     minMaxSpeed;
	XMFLOAT3     gravity; 
	float        drag; 
	XMFLOAT3     velocitySpread; 
	float        emitterLifetime; 
	float        particleLifetime; 
	float        spawnRadius; 
	bool         burst; 
	bool		 followEmitter;
	XMFLOAT3	 startColorMultiplierRGBMin; 
	XMFLOAT3	 startColorMultiplierRGBMax; 
	XMFLOAT3	 endColorMultiplierRGBMin; 
	XMFLOAT3	 endColorMultiplierRGBMax; 
	float        startAlpha; 
	float        endAlpha; 
	BLEND_STATE  BLEND;
	XMFLOAT3     spawnOffset; 
	bool		 LocalSpace;
	XMFLOAT2     startScaleMinMax; 
	XMFLOAT2     endScaleMinMax; 
	bool         rotationByVelocity;
	XMFLOAT2     rotationPerSecMinMax;
	XMFLOAT3     inheritVelocityScale;
};

class ParticleSystemComponent : public IComponent
{
public:	
	ParticleSystemComponent();	
	~ParticleSystemComponent();

	// will read the particle settings and create memory for all buffers
	void Init(const char* particleFile);
	void Init(std::vector<ParticleSettings> settings);

	// updates the simulation
	void Update(const float& delta);

	// uploads the buffers to the gpu
	void UploadBuffers(unsigned int index);

	// get the blendstate by emitter index in this system
	BLEND_STATE GetBlendState(int index) { return _settings[index].BLEND; }

	// get texture by emitter index in this system
	ID3D11ShaderResourceView* GetTexture(int index) { return textures[index]; }

	// get num particles by emitter index in this system
	unsigned int GetNumParticles(int index) { return _settings[index].numParticles; }

	// get particle settings by emitter index
	ParticleSettings GetSettings(int index){ return _settings[index]; }

	// get the position of the system transform
	XMFLOAT3 GetSystemPosition() { return _transform->position; }

	// how many emitter does this system have
	unsigned int numEmitters;

	// texture array, one per emitter in system
	ID3D11ShaderResourceView** textures;

private:

	// will allocate memory for all particle data
	void AllocateMemory();

	// will setup all emitters depending on settings
	void SetUp();

	// create all buffers for the emitters
	void CreateBuffers(XMFLOAT2 size, unsigned int index);

	// update the buffers before rendering
	void UpdateBuffer();

	// spawn all particles att the same time
	void SpawnAllParticles(unsigned int index);

	// spawn one particle
	void SpawnParticle(ParticleData& particle, unsigned int index);

	// update the simulation
	void UpdateVelocity(const float& delta);
	void UpdateLifeTime(const float& delta);
	void UpdateLerps(const float& delta);
	void UpdateRotations(const float& delta);
	void SortParticles(unsigned int index);

	// parse settings from .json file
	void ParsefromJson(const char* file);

	// helper functions
	float    GetRandomFloat(float min, float max);
	float    LerpFloat(float a, float b, float f);
	XMFLOAT3 GetDirectionLocal(XMFLOAT3 direction);
		
	// does this system have a lifetime
	// and should be destroyed after the time
	bool _hasLifetime;

	// array of particle settings
	// one per emitter
	ParticleSettings* _settings;

	// array of data per emitter basis
	EmitterData* _emitterData;

	// array of buffers
	// one per emitter
	ID3D11Buffer** _vertexBuffer;
	ID3D11Buffer** _indexBuffer;
	ID3D11Buffer** _instanceBuffer;

	// 2d array of data per emitter/per particle
	ParticleInstanceType** _particleInstanceData;
	ParticleData**         _particleData;

	// last position of system
	XMFLOAT3 _previousPosition;

	// pointer to entity transform
	TransformComponent* _transform;	

};

