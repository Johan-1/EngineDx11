#include "ImGUIShader.h"
#include "DXManager.h"
#include "DXBlendStates.h"
#include "DXDepthStencilStates.h"
#include "DXRasterizerStates.h"
#include "imgui.h"
#include "GUIManager.h"
#include "ShaderHelpers.h"
#include "DXErrorHandler.h"
#include "Systems.h"

ImGUIShader::ImGUIShader()
{
	// create shaders
	SHADER_HELPERS::CreateVertexShader(L"Data/shaders/vertexImGui.shader", _vertexShader, _vertexShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"Data/shaders/pixelImGui.shader",   _pixelShader,  _pixelShaderByteCode);

	// create constant buffers
	SHADER_HELPERS::CreateConstantBuffer(_constantBufferVertex);
}

ImGUIShader::~ImGUIShader()
{
	_vertexShaderByteCode->Release();
	_pixelShaderByteCode->Release();

	_vertexShader->Release();
	_pixelShader->Release();

	_constantBufferVertex->Release();
}

void ImGUIShader::RenderGUI()
{
	// setup ImGui buffers before rendering
	ImGui::Render();

	// get DX manager
	DXManager& DXM = *Systems::dxManager;

	// get GUI manager
	GuiManager& GUIM = *Systems::guiManager;

	// get device and device context
	ID3D11DeviceContext*& devCon = DXM.devCon;
	ID3D11Device*& device        = DXM.device;

	// get the vertex and index buffers from IM GUI
	ID3D11Buffer*& vertexBuffer = GUIM.vertexBuffer;
	ID3D11Buffer*& indexBuffer  = GUIM.indexBuffer;

	// get the draw data from IM GUI
	ImDrawData* draw_data = ImGui::GetDrawData();

	HRESULT result;

	// disable depth and set to alpha blending
	DXM.blendStates->SetBlendState(BLEND_STATE::BLEND_ALPHA);
	DXM.depthStencilStates->SetDepthStencilState(DEPTH_STENCIL_STATE::DISABLED);

	// declare vertex and index buffer descriptions
	D3D11_BUFFER_DESC currentSizevertex;
	D3D11_BUFFER_DESC currentSizeIndex;

	// get the current descriptions of the vertex and index buffers
	vertexBuffer->GetDesc(&currentSizevertex);
	indexBuffer->GetDesc(&currentSizeIndex);

	// check if the current size of the vertex buffer is smaller
	// then the size needed to render all GUI, if it is we release the old 
	// buffer and create a new bigger one
	if (currentSizevertex.ByteWidth < draw_data->TotalVtxCount * sizeof(ImDrawVert))
	{
		// release old vertex buffer
		vertexBuffer->Release();
		vertexBuffer = nullptr;

		// set the description for the new buffer
		// just increase the size of the buffer with an arbitrary amount
		D3D11_BUFFER_DESC desc;
		desc.Usage          = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth      = currentSizevertex.ByteWidth + (5000 * sizeof(ImDrawVert));
		desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags      = 0;

		// create the buffer
		result = device->CreateBuffer(&desc, NULL, &vertexBuffer);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to upgrade the size of the vertex GUI buffer");
	}

	// check if the current size of the index buffer is smaller
	// then the size needed to render all GUI, if it is we release the old 
	// buffer and create a new bigger one
	if (currentSizeIndex.ByteWidth < draw_data->TotalIdxCount * sizeof(ImDrawIdx))
	{
		// release old index buffer
		indexBuffer->Release();
		indexBuffer = nullptr;

		// set the description for the new buffer
		// just increase the size of the buffer with an arbitrary amount
		D3D11_BUFFER_DESC desc;
		desc.Usage          = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth      = currentSizeIndex.ByteWidth + (10000 * sizeof(ImDrawVert));
		desc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags      = 0;

		// create the buffer
		result = device->CreateBuffer(&desc, NULL, &indexBuffer);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to upgrade the size of the index GUI buffer");
	}

	// sub resources to our bound buffers
	D3D11_MAPPED_SUBRESOURCE vtx_resource, idx_resource;

	// map vertex buffer
	result = devCon->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource);
	if (FAILED(result))
		DX_ERROR::PrintError(result, "failed to map the IM GUI vertex buffer");

	// map index buffer
	result = devCon->Map(indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource);
	if (FAILED(result))
		DX_ERROR::PrintError(result, "failed to map the IM GUI index buffer");

	// get the adress to where the buffers are mapped so
	// we can write our data
	ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource.pData;
	ImDrawIdx*  idx_dst = (ImDrawIdx*)idx_resource.pData;

	// loop over all IM GUI command lists and upload the data to our buffers
	for (int i = 0; i < draw_data->CmdListsCount; i++)
	{
		// get ponter to IM GUI command list
		const ImDrawList* cmd_list = draw_data->CmdLists[i];

		// upload the data to the buffers
		memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

		// move our pointers forward so we will start writing
		// at the location where the data of this command list ended
		vtx_dst += cmd_list->VtxBuffer.Size;
		idx_dst += cmd_list->IdxBuffer.Size;
	}

	// unmap buffers
	devCon->Unmap(vertexBuffer, 0);
	devCon->Unmap(indexBuffer, 0);

	// create worldViewProjection
	float L = 0.0f;
	float R = ImGui::GetIO().DisplaySize.x;
	float B = ImGui::GetIO().DisplaySize.y;
	float T = 0.0f;
	float wvp[4][4] =
	{
		{ 2.0f / (R - L),    0.0f,              0.0f,       0.0f },
		{ 0.0f,              2.0f / (T - B),    0.0f,       0.0f },
		{ 0.0f,              0.0f,              0.5f,       0.0f },
		{ (R + L) / (L - R), (T + B) / (B - T), 0.5f,       1.0f },
	};

	// copy our array in the the constant vertex structure
	ConstantVertex constantVertex;
	memcpy(&constantVertex.projection, wvp, sizeof(wvp));

	// update the constant vertex buffer
	SHADER_HELPERS::UpdateConstantBuffer(&constantVertex, sizeof(ConstantVertex), _constantBufferVertex);

	// set stride and offset
	unsigned int stride = sizeof(ImDrawVert);
	unsigned int offset = 0;

	// set vertex and index buffers
	devCon->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	devCon->IASetIndexBuffer(indexBuffer, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);

	// set shaders
	devCon->VSSetShader(_vertexShader, NULL, 0);
	devCon->PSSetShader(_pixelShader, NULL, 0);

	// set constant buffer
	devCon->VSSetConstantBuffers(0, 1, &_constantBufferVertex);

	// draw everything
	int vtx_offset = 0;
	int idx_offset = 0;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];		
			const D3D11_RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
			devCon->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&pcmd->TextureId);
			devCon->RSSetScissorRects(1, &r);
			devCon->DrawIndexed(pcmd->ElemCount, idx_offset, vtx_offset);			
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.Size;
	}

	// enable deapth
	DXM.depthStencilStates->SetDepthStencilState(DEPTH_STENCIL_STATE::ENABLED);
}