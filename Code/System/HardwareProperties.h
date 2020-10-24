#pragma once
#include <D3D11.h>

// structure to keep info about our hardware
struct HardwareInfo
{
public:
	unsigned int  numerator;
	unsigned int  denominator;
	unsigned int  refreshrate;
	unsigned int  videoCardMemory;
	char          videoCardDescription[128];
};

class HardwareProperties
{
public:
	HardwareProperties(int screenWidth, int screenHeight);
	~HardwareProperties();

	// stores hardware info
	HardwareInfo hardwareInfo;

private:

	// gets hardware info
	void GetHardwareInfo(int screenWidth, int screenHeight);
};

