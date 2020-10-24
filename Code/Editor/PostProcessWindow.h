#pragma once
#include "IEditorWindow.h"

class MasterEditor;

class PostProcessWindow : public IEditorWindow
{
public:
	PostProcessWindow(MasterEditor* masterEditor);
	~PostProcessWindow();

	void Render();

private:

};

