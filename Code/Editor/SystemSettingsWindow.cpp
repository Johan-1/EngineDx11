#include "SystemSettingsWindow.h"
#include "GUI.h"
#include "SystemDefs.h"
#include "MasterEditor.h"

SystemSettingsWindow::SystemSettingsWindow(MasterEditor* masterEditor) : IEditorWindow(masterEditor)
{
	// keep a copy of these values to change and then save them to file
	// so they will be used at next application start
	_settings.screenSize[0] = (int)SystemSettings::SCREEN_WIDTH;
	_settings.screenSize[1] = (int)SystemSettings::SCREEN_HEIGHT;
	_settings.vSync         = SystemSettings::V_SYNC;
	_settings.fullScreen    = SystemSettings::FULLSCREEN;
	_settings.HDR           = SystemSettings::USE_HDR;
}

SystemSettingsWindow::~SystemSettingsWindow()
{
}

void SystemSettingsWindow::Render()
{
	GUI::BeginWindow("System Settings", 0.6f, SystemSettings::SCREEN_WIDTH * 0.20f, SystemSettings::SCREEN_HEIGHT * 0.5f, 0, 0, 0, 0);

	GUI::Space(1);
	GUI::Text(ImVec4(1, 1, 1, 1), "RESTART IS NEEDED FOR THESE SETTINGS TO TAKE AFFECT");
	GUI::Space(2);
	GUI::IntInput2("1", "Screen Size", "the size of the screen",                   &_settings.screenSize[0]);
	GUI::CheckBox("2",  "V Sync",      "if vertical sync should be used",          &_settings.vSync);
	GUI::CheckBox("3",  "Fullscreen",  "if fullscreen mode should be used",        &_settings.fullScreen);
	GUI::CheckBox("4",  "Use HDR",     "if We will render to a HDR render target", &_settings.HDR);

	GUI::Space(1);
	if (GUI::Button("Save settings"))
	{
		std::string file = GUI::FindFileFromDirectory(".json", "Save settings to file");
		if (file != "")
			SystemSettings::WriteSettings(file.c_str(), _settings);		
	}

	GUI::Space(1);
	if (GUI::Button("Back"))
		GoToMain();

	GUI::EndWindow();
}