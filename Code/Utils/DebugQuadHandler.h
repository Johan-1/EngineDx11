#pragma once
#include <vector>
#include <d3d11.h>

class Entity;

class DebugQuadHandler
{
public:
	DebugQuadHandler();
	~DebugQuadHandler();

	void AddDebugQuad(ID3D11ShaderResourceView* texture);
	void RemoveDebugQuad(ID3D11ShaderResourceView* texture);
	void ReplaceTexture(ID3D11ShaderResourceView* original, ID3D11ShaderResourceView* replacement);

private:

	int   _count;
	int   _maxPerRow;
	float _width;
	float _height;
	float _spacingHorizontal;
	float _spacingVertical;
	float _nextStartPosX;
	float _nextStartPosY;

	std::vector<Entity*> _quadEntities;
};


