#pragma once
#include "IEditorWindow.h"

class MasterEditor;

class SystemSettingsWindow : public IEditorWindow
{
public:
	SystemSettingsWindow(MasterEditor* masterEditor);
	virtual ~SystemSettingsWindow();

	void Render();

	struct Settings
	{
		int screenSize[2];
		bool vSync;
		bool fullScreen;
		bool HDR;
	};

private:
	Settings _settings;
};


