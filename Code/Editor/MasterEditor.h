#pragma once
#include "IEditorWindow.h"
#include "MainEditorWindow.h"
#include "SkyDomeWindow.h"
#include "PostProcessWindow.h"
#include "SystemSettingsWindow.h"

class MasterEditor
{
public:
	MasterEditor();
	~MasterEditor();

	void Update();
	void RenderStatsWindow();
	void RenderEditorWindow();

	IEditorWindow*        activeWindow;
	MainEditorWindow*     mainWindow;
	SkyDomeWindow*        skyDomeWindow;
	PostProcessWindow*    postProcessWindow;
	SystemSettingsWindow* systemSettingsWindow;

private:

	bool _showEditorWindow;
	bool _showStatsWindow;
};

