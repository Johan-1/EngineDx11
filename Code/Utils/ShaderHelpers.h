#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include "DXManager.h"
#include "DXErrorHandler.h"
#include <vector>
#include "Mesh.h"
#include "Systems.h"

namespace SHADER_HELPERS
{
	static void CreateConstantBuffer(ID3D11Buffer*& buffer)
	{
		ID3D11Device*& device = Systems::dxManager->device;

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth           = 16;
		bufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags           = 0;
		bufferDesc.StructureByteStride = 0;

		HRESULT result;

		result = device->CreateBuffer(&bufferDesc, NULL, &buffer);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to create new constant buffer");
	}

	static void CreateStructuredStagingBuffer(unsigned int structSize, unsigned int count, ID3D11Buffer*& buffer)
	{
		ID3D11Device*& device = Systems::dxManager->device;

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage               = D3D11_USAGE_STAGING;
		bufferDesc.ByteWidth           = structSize * count;
		bufferDesc.BindFlags           = 0;
		bufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_READ;
		bufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bufferDesc.StructureByteStride = structSize;

		HRESULT result;

		result = device->CreateBuffer(&bufferDesc, NULL, &buffer);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to create new structured staging buffer buffer");
	}

	static void CreateStructuredSRVBuffer(void* data, unsigned int structSize, unsigned int count, ID3D11Buffer*& buffer, ID3D11ShaderResourceView*& bufferSRV)
	{
		ID3D11Device*& device = Systems::dxManager->device;

		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth           = structSize * count;
		bufferDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bufferDesc.StructureByteStride = structSize;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format              = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension       = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements  = count;

		HRESULT result;

		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = data;

		result = device->CreateBuffer(&bufferDesc, &subResource, &buffer);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to create new structured buffer");

		result = device->CreateShaderResourceView(buffer, &srvDesc, &bufferSRV);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to create SRV for structured buffer");
	}

	static void CreateStructuredUAVSRVBuffer(void* initialData, unsigned int structSize, unsigned int count, ID3D11Buffer*& buffer, ID3D11ShaderResourceView*& bufferSRV, ID3D11UnorderedAccessView*& bufferUAV, bool append = false)
	{
		ID3D11Device*& device = Systems::dxManager->device;

		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.Usage               = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth           = structSize * count;
		bufferDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		bufferDesc.CPUAccessFlags      = 0;
		bufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bufferDesc.StructureByteStride = structSize;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format              = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension       = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements  = count;

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
		uavDesc.Format              = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.Flags        = append ? D3D11_BUFFER_UAV_FLAG_APPEND : 0;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements  = count;

		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = initialData;

		HRESULT result;
		result = device->CreateBuffer(&bufferDesc, &subResource, &buffer);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to create new structured buffer");

		result = device->CreateUnorderedAccessView(buffer, &uavDesc, &bufferUAV);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to create UAV for structured buffer");

		result = device->CreateShaderResourceView(buffer, &srvDesc, &bufferSRV);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to create SRV for structured buffer");
	}

	static void CreateTexture2DUAVSRV(int width, int height, ID3D11Texture2D*& texture, ID3D11ShaderResourceView*& texSRV, ID3D11UnorderedAccessView*& texUAV)
	{
		ID3D11Device*& device = Systems::dxManager->device;

		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width              = width;
		texDesc.Height             = height;
		texDesc.MipLevels          = 1;
		texDesc.ArraySize          = 1;
		texDesc.SampleDesc.Count   = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage              = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags          = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		texDesc.Format             = DXGI_FORMAT_R16G16B16A16_FLOAT;

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));
		uavDesc.Format             = DXGI_FORMAT_R16G16B16A16_FLOAT;
		uavDesc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		//the getSRV function after dispatch.
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format              = DXGI_FORMAT_R16G16B16A16_FLOAT;
		srvDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		HRESULT result;
		result = device->CreateTexture2D(&texDesc, NULL, &texture);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to create new structured buffer");

		result = device->CreateUnorderedAccessView(texture, &uavDesc, &texUAV);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to create new structured buffer");

		result = device->CreateShaderResourceView(texture, &srvDesc, &texSRV);
		if (FAILED(result))
			DX_ERROR::PrintError(result, "failed to create new structured buffer");
	}

	static void UpdateConstantBuffer(void* data, unsigned int size, ID3D11Buffer*& buffer)
	{
		ID3D11DeviceContext*& devCon = Systems::dxManager->devCon;
		ID3D11Device*& device        = Systems::dxManager->device;

		//get desc of the current constant buffer in use
		D3D11_BUFFER_DESC old;
		buffer->GetDesc(&old);

		// if smaller then the new data we create a new bigger one and remove the old
		if (old.ByteWidth < size)
		{
			buffer->Release();
			buffer = nullptr;

			HRESULT result = 0;
			D3D11_BUFFER_DESC bufferDesc;

			bufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth           = size;
			bufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags           = 0;
			bufferDesc.StructureByteStride = 0;

			result = device->CreateBuffer(&bufferDesc, NULL, &buffer);
			if (FAILED(result))
				DX_ERROR::PrintError(result, "failed to create new constant buffer when upgrading size");
		}

		// uppdate the buffer with the new data
		D3D11_MAPPED_SUBRESOURCE subResource;
		devCon->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);

		memcpy(subResource.pData, data, size);

		devCon->Unmap(buffer, 0);
	}

	static void UpdateStructuredBuffer(void* data, unsigned int structSize, unsigned int count, ID3D11Buffer*& buffer)
	{
		ID3D11DeviceContext*& devCon = Systems::dxManager->devCon;
		ID3D11Device*&        device = Systems::dxManager->device;

		//get desc of the current constant buffer in use
		D3D11_BUFFER_DESC old;
		buffer->GetDesc(&old);

		// if smaller then the new data we create a new bigger one and remove the old
		if (old.ByteWidth < (structSize * count))
		{
			buffer->Release();
			buffer = nullptr;

			HRESULT result = 0;
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth           = structSize * count;
			bufferDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
			bufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bufferDesc.StructureByteStride = structSize;

			result = device->CreateBuffer(&bufferDesc, NULL, &buffer);
			if (FAILED(result))
				DX_ERROR::PrintError(result, "failed to create new structured buffer while upgrading size");
		}

		// uppdate the buffer with the new data
		D3D11_MAPPED_SUBRESOURCE subResource;
		devCon->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);

		memcpy(subResource.pData, data, (structSize * count));

		devCon->Unmap(buffer, 0);
	}

	static void CreateVertexShader(LPCWSTR filePath, ID3D11VertexShader*& shader, ID3D10Blob*& buffer, char* entryPoint = "Main")
	{
		ID3D11Device*& device = Systems::dxManager->device;
		ID3D10Blob* errorMessage = nullptr;

		// compile the vertex shader code from the text file into temporal buffers
		HRESULT result = D3DCompileFromFile(filePath, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, "vs_5_0", 0, 0, &buffer, &errorMessage);
		if (FAILED(result))	DX_ERROR::PrintError(result, (std::string("failed to compile Vertex shader : ") + DX_ERROR::ConvertFromWString((wchar_t*)filePath)).c_str(), errorMessage);
	
		// once the code have been compiled into the buffer we can create the shader objects themselfs from the blob objects
		result = device->CreateVertexShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), NULL, &shader);
		if (FAILED(result)) DX_ERROR::PrintError(result, (std::string("failed to create vertex shader from ID3D10Blob, shader file = : ") + DX_ERROR::ConvertFromWString((wchar_t*)filePath)).c_str());
	}

	static void CreatePixelShader(LPCWSTR filePath, ID3D11PixelShader*& shader, ID3D10Blob*& buffer, char* entryPoint = "Main")
	{
		ID3D11Device*& device = Systems::dxManager->device;
		ID3D10Blob* errorMessage = nullptr;

		// compile the pixel shader code from the text file into temporal buffers
		HRESULT result = D3DCompileFromFile(filePath, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, "ps_5_0", 0, 0, &buffer, &errorMessage);
		if (FAILED(result)) DX_ERROR::PrintError(result, (std::string("failed to compile Pixel shader : ") + DX_ERROR::ConvertFromWString((wchar_t*)filePath)).c_str(), errorMessage);
					
		// create shader from blob
		result = device->CreatePixelShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), NULL, &shader);
		if (FAILED(result)) DX_ERROR::PrintError(result, (std::string("failed to create pixel shader from ID3D10Blob, shader file = : ") + DX_ERROR::ConvertFromWString((wchar_t*)filePath)).c_str());
	}

	static void CreateComputeShader(LPCWSTR filePath, ID3D11ComputeShader*& shader, ID3D10Blob*& buffer, char* entryPoint = "Main")
	{

		ID3D11Device*& device = Systems::dxManager->device;
		ID3D10Blob* errorMessage = nullptr;

		// compile the compute shader code into blob
		HRESULT result = D3DCompileFromFile(filePath, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, "cs_5_0", 0, 0, &buffer, &errorMessage);
		if (FAILED(result)) DX_ERROR::PrintError(result, (std::string("failed to compile compute shader : ") + DX_ERROR::ConvertFromWString((wchar_t*)filePath)).c_str(), errorMessage);		

		// create shader from blob
		result = device->CreateComputeShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), NULL, &shader);
		if (FAILED(result)) DX_ERROR::PrintError(result, (std::string("failed to create compute shader from ID3D10Blob, shader file = : ") + DX_ERROR::ConvertFromWString((wchar_t*)filePath)).c_str());
	}

	static void MeshSort(std::vector<Mesh*>& meshes, const XMFLOAT3& position, bool backToFront)
	{
		const int size = (int)meshes.size();

		for (int i = 0; i < size; i++)
		{
			XMFLOAT3 vec;
			float distance = 0;

			XMStoreFloat3(&vec, XMVectorSubtract(XMLoadFloat3(&meshes[i]->GetPosition()), XMLoadFloat3(&position)));
			XMStoreFloat(&distance, XMVector3Length(XMLoadFloat3(&vec)));

			meshes[i]->camDistance = distance;
		}

		if (backToFront)
			std::sort(meshes.begin(), meshes.end(), [](Mesh* a, Mesh* b) -> bool {return a->camDistance > b->camDistance; });
		else
			std::sort(meshes.begin(), meshes.end(), [](Mesh* a, Mesh* b) -> bool {return a->camDistance < b->camDistance; });
	}
}