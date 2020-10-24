#include "IEditorWindow.h"
#include "MasterEditor.h"

IEditorWindow::IEditorWindow(MasterEditor* masterEditor):
	_masterEditor(masterEditor)
{
}

IEditorWindow::~IEditorWindow()
{
}

void IEditorWindow::GoToMain()
{
	_masterEditor->activeWindow = _masterEditor->mainWindow;
}
