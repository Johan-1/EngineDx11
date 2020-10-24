#pragma once
#include "IEditorWindow.h"
#include <DirectXMath.h>
#include <string>

using namespace DirectX;
class MasterEditor;
struct SkySettings;
struct SunMoon;

class SkyDomeWindow : public IEditorWindow
{
public:
	SkyDomeWindow(MasterEditor* masterEditor);
	virtual ~SkyDomeWindow();

	void Render();

	void RenderColorBlend(SkySettings* skySettings);
	void RenderCubeMap(SkySettings* skySettings);
	void RenderLightning(SkySettings* skySettings);
	void RenderSunMoon(SunMoon* sunMoon, SkySettings* skySettings);
	
	void SaveSettings(const char* file, SkySettings* skySettings, SunMoon* sunMoon);
};

