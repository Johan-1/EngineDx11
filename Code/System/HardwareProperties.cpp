#include "HardwareProperties.h"
#include <iostream>
#include "DXErrorHandler.h"

HardwareProperties::HardwareProperties(int screenWidth, int screenHeight)
{
	GetHardwareInfo(screenWidth, screenHeight);
}

HardwareProperties::~HardwareProperties()
{
}

void HardwareProperties::GetHardwareInfo(int screenWidth, int screenHeight)
{
	HRESULT result;
	IDXGIFactory* factory;       // used to create grapichal interfaces
	IDXGIAdapter* adapter;       // use the adapter to get acces to certain hardware properties
	IDXGIOutput*  adapterOutput; // give access to our output devices

	unsigned int numModes;
	size_t stringLenght;
	DXGI_MODE_DESC* displayModeList; // store all modes in here
	DXGI_ADAPTER_DESC adapterDesc;   // store the description of our videocard here
	int error;

	// Create factory
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create DXGI factory");

	// use the factory to create a adapter for the primary graphics interface(video card), we can then use the adapter to get info about our monitor/video memory etc
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create adapter");

	// get the primary adapter output
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create adapter output");

	//get the number of modes that fit the DXGI_FORMATR8G8B8A8_UNORM display format for the adapter output
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to get display modes");

	// create a list to hold all the possible modes for this monitor/videocard combination
	displayModeList = new DXGI_MODE_DESC[numModes];

	// get all displaymodes
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create output mode list");

	// save the monitors refreshrate to be used if Vsync is enabled	
	for (int i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth && displayModeList[i].Height == (unsigned int)screenHeight)
		{
			hardwareInfo.numerator   = displayModeList[i].RefreshRate.Numerator;
			hardwareInfo.denominator = displayModeList[i].RefreshRate.Denominator;
			hardwareInfo.refreshrate = (int)round((float)hardwareInfo.numerator / hardwareInfo.denominator);
		}
	}
	if (hardwareInfo.numerator == 0 && hardwareInfo.denominator == 0)
		printf("your monitor do not support the current aspect ratio\n");

	// Get the adapter description 
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
		DX_ERROR::PrintError(result, "failed to create adapter description");

	// store the videocard memory in mbs
	hardwareInfo.videoCardMemory = (unsigned int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// convert the name of videocard to character array
	error = wcstombs_s(&stringLenght, hardwareInfo.videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
		printf("Failed to convert the videocard info");

	// cleanup
	delete[] displayModeList;
	adapter->Release();
	factory->Release();
	adapterOutput->Release();
}
