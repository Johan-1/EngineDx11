#pragma once
#include "IEditorWindow.h"

class MainEditorWindow : public IEditorWindow
{
public:
	MainEditorWindow(MasterEditor* masterEditor);
	virtual ~MainEditorWindow();

	void Render();
};

