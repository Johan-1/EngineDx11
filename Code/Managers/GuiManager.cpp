#include "GuiManager.h"
#include "DXManager.h"
#include <iostream>
#include "Input.h"
#include "Time.h"
#include "Systems.h"
#include "DXErrorhandler.h"

GuiManager::GuiManager()
{	
}

GuiManager::~GuiManager()
{
}

void GuiManager::Initialize(HWND hwnd)
{
	_hwnd = hwnd;

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// Setup back-end capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;  // We can honor io.WantSetMousePos requests (optional, rarely used)	
			
	// set the IMGUI io keys to the the ID's of direct input representations
	io.KeyMap[ImGuiKey_Tab]        = DIK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow]  = DIK_LEFTARROW;
	io.KeyMap[ImGuiKey_RightArrow] = DIK_RIGHTARROW;
	io.KeyMap[ImGuiKey_UpArrow]    = DIK_UPARROW;
	io.KeyMap[ImGuiKey_DownArrow]  = DIK_DOWNARROW;
	io.KeyMap[ImGuiKey_PageUp]     = DIK_PGUP;
	io.KeyMap[ImGuiKey_PageDown]   = DIK_PGDN;
	io.KeyMap[ImGuiKey_Home]       = DIK_HOME;
	io.KeyMap[ImGuiKey_End]        = DIK_END;
	io.KeyMap[ImGuiKey_Insert]     = DIK_INSERT;
	io.KeyMap[ImGuiKey_Delete]     = DIK_DELETE;
	io.KeyMap[ImGuiKey_Backspace]  = DIK_BACKSPACE;
	io.KeyMap[ImGuiKey_Space]      = DIK_SPACE;
	io.KeyMap[ImGuiKey_Enter]      = DIK_RETURN;
	io.KeyMap[ImGuiKey_Escape]     = DIK_ESCAPE;
	io.KeyMap[ImGuiKey_A]          = DIK_A;
	io.KeyMap[ImGuiKey_C]          = DIK_C;
	io.KeyMap[ImGuiKey_V]          = DIK_V;
	io.KeyMap[ImGuiKey_X]          = DIK_X;
	io.KeyMap[ImGuiKey_Y]          = DIK_Y;
	io.KeyMap[ImGuiKey_Z]          = DIK_Z;
		
	// set window handle
	io.ImeWindowHandle = _hwnd;

	// create textures and buffers
	CreateTexture();
	CreateBuffers();
}

void GuiManager::CreateTexture()
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	ID3D11Device*& device = Systems::dxManager->device;

	// get texture data from IMGUI
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		
	// create texture description
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width            = width;
	desc.Height           = height;
	desc.MipLevels        = 1;
	desc.ArraySize        = 1;
	desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage            = D3D11_USAGE_DEFAULT;
	desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags   = 0;

	ID3D11Texture2D       *texture = NULL;
	D3D11_SUBRESOURCE_DATA subResource;

	// create the texture with the pixel data
	subResource.pSysMem          = pixels;
	subResource.SysMemPitch      = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	device->CreateTexture2D(&desc, &subResource, &texture);

	// Create SRV to use
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels       = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	device->CreateShaderResourceView(texture, &srvDesc, &_texture);
	texture->Release();
	
	// Store our identifier
	io.Fonts->TexID = (void *)_texture;
}

void GuiManager::CreateBuffers()
{	
	// get the device
	ID3D11Device*& device = Systems::dxManager->device;
		
	// vertexbuffer desc
	D3D11_BUFFER_DESC vertexDesc;
	memset(&vertexDesc, 0, sizeof(D3D11_BUFFER_DESC));
	vertexDesc.Usage          = D3D11_USAGE_DYNAMIC;
	vertexDesc.ByteWidth      = 5000 * sizeof(ImDrawVert);
	vertexDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexDesc.MiscFlags      = 0;
	
	// indexbuffer desc
	D3D11_BUFFER_DESC indexDesc;
	memset(&indexDesc, 0, sizeof(D3D11_BUFFER_DESC));
	indexDesc.Usage          = D3D11_USAGE_DYNAMIC;
	indexDesc.ByteWidth      = 10000 * sizeof(ImDrawIdx);
	indexDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
	indexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT result;

	result = device->CreateBuffer(&vertexDesc, NULL, &vertexBuffer);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create vertex buffer for GUI");

	result = device->CreateBuffer(&indexDesc, NULL, &indexBuffer);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create index buffer for GUI");
}

void GuiManager::Update()
{
	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	RECT rect;
	GetClientRect(_hwnd, &rect);
	io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

	ImGui::GetIO().DeltaTime = Systems::time->GetDeltaTime();

	if (io.WantSetMousePos)
	{
		POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
		ClientToScreen(_hwnd, &pos);
		SetCursorPos(pos.x, pos.y);
	}

	// Update OS mouse cursor with the cursor requested by imgui
	ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
	if (_lastMouseCursor != mouse_cursor)
	{
		_lastMouseCursor = mouse_cursor;
		UpdateMouseCursor();		
	}
	
	// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
	ImGui::NewFrame();
}

void GuiManager::UpdateMouseCursor()
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_NoSetMouseCursor)
		return;

	ImGuiMouseCursor imgui_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
	if (imgui_cursor == ImGuiMouseCursor_None)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		::SetCursor(NULL);
	}
	else
	{
		// Hardware cursor type
		LPTSTR win32_cursor = IDC_ARROW;
		switch (imgui_cursor)
		{
		case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW;    break;
		case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM;    break;
		case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL;  break;
		case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE;   break;
		case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS;   break;
		case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
		case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
		}
		::SetCursor(::LoadCursor(NULL, win32_cursor));
	}
}



