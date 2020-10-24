#pragma once
#include <DirectXMath.h>

using namespace DirectX;

namespace MATH_HELPERS
{
	static XMFLOAT4X4 MatrixMutiplyTrans(const XMFLOAT4X4* a, const XMFLOAT4X4* b, const XMFLOAT4X4* c)
	{
		XMFLOAT4X4 result;

		XMStoreFloat4x4(&result, XMMatrixMultiply(XMLoadFloat4x4(a),
			XMMatrixMultiply(XMLoadFloat4x4(b), XMLoadFloat4x4(c))));

		XMStoreFloat4x4(&result, XMMatrixTranspose(XMLoadFloat4x4(&result)));

		return result;
	}

	static XMFLOAT4X4 MatrixMutiplyTrans(const XMFLOAT4X4* a, const XMFLOAT4X4* b)
	{
		XMFLOAT4X4 result;

		XMStoreFloat4x4(&result, XMMatrixMultiply(XMLoadFloat4x4(a), XMLoadFloat4x4(b)));
		XMStoreFloat4x4(&result, XMMatrixTranspose(XMLoadFloat4x4(&result)));
			
		return result;
	}

	static XMFLOAT4X4 MatrixMutiply(const XMFLOAT4X4* a, const XMFLOAT4X4* b, const XMFLOAT4X4* c)
	{
		XMFLOAT4X4 result;

		XMStoreFloat4x4(&result, XMMatrixMultiply(XMLoadFloat4x4(a),
			XMMatrixMultiply(XMLoadFloat4x4(b), XMLoadFloat4x4(c))));

		return result;
	}

	static XMFLOAT4X4 MatrixMutiply(const XMFLOAT4X4* a, const XMFLOAT4X4* b)
	{
		XMFLOAT4X4 result;

		XMStoreFloat4x4(&result, XMMatrixMultiply(XMLoadFloat4x4(a), XMLoadFloat4x4(b)));

		return result;
	}

	static XMFLOAT4X4 CreateWorldMatrix(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale)
	{
		XMFLOAT3 rotRadian(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));

		XMFLOAT4X4 positionMatrix;
		XMFLOAT4X4 rotationMatrix;
		XMFLOAT4X4 scaleMatrix;

		XMFLOAT4X4 result;

		XMStoreFloat4x4(&positionMatrix, XMMatrixTranslationFromVector(XMLoadFloat3(&position)));
		XMStoreFloat4x4(&scaleMatrix,    XMMatrixScalingFromVector(XMLoadFloat3(&scale)));
		XMStoreFloat4x4(&rotationMatrix, XMMatrixRotationRollPitchYaw(rotRadian.x, rotRadian.y, rotRadian.z));

		XMStoreFloat4x4(&result, XMMatrixMultiply(XMLoadFloat4x4(&scaleMatrix), XMLoadFloat4x4(&rotationMatrix)));
		XMStoreFloat4x4(&result, XMMatrixMultiply(XMLoadFloat4x4(&result),      XMLoadFloat4x4(&positionMatrix)));

		return result;
	}

	static XMFLOAT4X4 CreateWorldMatrixTrans(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale)
	{
		XMFLOAT3 rotRadian(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));

		XMFLOAT4X4 positionMatrix;
		XMFLOAT4X4 rotationMatrix;
		XMFLOAT4X4 scaleMatrix;

		XMFLOAT4X4 result;

		XMStoreFloat4x4(&positionMatrix, XMMatrixTranslationFromVector(XMLoadFloat3(&position)));
		XMStoreFloat4x4(&scaleMatrix,    XMMatrixScalingFromVector(XMLoadFloat3(&scale)));
		XMStoreFloat4x4(&rotationMatrix, XMMatrixRotationRollPitchYaw(rotRadian.x, rotRadian.y, rotRadian.z));

		XMStoreFloat4x4(&result, XMMatrixMultiply(XMLoadFloat4x4(&scaleMatrix), XMLoadFloat4x4(&rotationMatrix)));
		XMStoreFloat4x4(&result, XMMatrixMultiply(XMLoadFloat4x4(&result), XMLoadFloat4x4(&positionMatrix)));

		XMStoreFloat4x4(&result, XMMatrixTranspose(XMLoadFloat4x4(&result)));

		return result;
	}
}
