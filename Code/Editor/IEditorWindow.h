#pragma once

class MasterEditor;

class IEditorWindow
{
public:
	IEditorWindow(MasterEditor* masterEditor);
	virtual ~IEditorWindow();

	void GoToMain();
	virtual void Render() = 0;

protected:

	MasterEditor* _masterEditor;
};

