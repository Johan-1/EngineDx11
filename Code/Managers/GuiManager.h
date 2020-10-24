#pragma once
#include <D3D11.h>
#include "imgui.h"

class GuiManager
{
public:
	GuiManager();
	~GuiManager();

	void Update();
	void Initialize(HWND hwnd);
	void UpdateMouseCursor();
		
	// vertex and index buffers
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

private:
	
	// creates the buffers and texture
	void CreateBuffers();
	void CreateTexture();

	// texture for displaying the UI 
	ID3D11ShaderResourceView* _texture;

	// pointer to the window handle
	HWND _hwnd;

	ImGuiMouseCursor _lastMouseCursor = ImGuiMouseCursor_COUNT;
};

