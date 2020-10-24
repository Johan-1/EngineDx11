#pragma once
#include <d3d11.h>
#include <comdef.h>
#include <iostream>

namespace DX_ERROR
{
	// prints an error message from HRESULT with a message from the user
	static void PrintError(HRESULT result, const char* userMessage, ID3D10Blob* extraMsgData = nullptr)
	{
		_com_error err(result);
		LPCTSTR msg = err.ErrorMessage();

		char* extraMsg = extraMsgData != nullptr ? (char*)extraMsgData->GetBufferPointer() : "";
		printf("****************\nWARNING DIRECT X ERROR:\nUser message  - %s\nError message - %s\n%s\n*****************\n", userMessage, msg, extraMsg);
	}

	// convert from wide string to string
	static std::string ConvertFromWString(const wchar_t* wide)
	{
		_bstr_t conversion (wide);
		const char * text = conversion;

		return text;
	}
}