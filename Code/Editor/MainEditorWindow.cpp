#include "MainEditorWindow.h"
#include "SystemDefs.h"
#include "imgui.h"
#include "MasterEditor.h"
#include "GUI.h"

MainEditorWindow::MainEditorWindow(MasterEditor* masterEditor) : IEditorWindow(masterEditor)
{
}

MainEditorWindow::~MainEditorWindow()
{
}

void MainEditorWindow::Render()
{
	GUI::BeginWindow("Editor", 0.6f, SystemSettings::SCREEN_WIDTH * 0.15f, SystemSettings::SCREEN_HEIGHT, 0, 0, 0, 0);
	
	if (GUI::Button("System Settings"))
		_masterEditor->activeWindow = _masterEditor->systemSettingsWindow;

	GUI::Space(1);

	if (GUI::Button("Post Processing Settings"))
		_masterEditor->activeWindow = _masterEditor->postProcessWindow;

	GUI::Space(1);

	if (GUI::Button("SkyDome Settings"))
		_masterEditor->activeWindow = _masterEditor->skyDomeWindow;


	GUI::Text(ImVec4(1, 1, 1, 1), "Press F1 to toggle GameCamera");
	GUI::Text(ImVec4(1, 1, 1, 1), "Press F2 to toggle ShadowCamera");

	// end rendering of this window
	GUI::EndWindow();
}