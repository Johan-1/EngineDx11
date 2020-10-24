#pragma once
#include <Windows.h>
class Window
{
public:
	Window(const char* title, unsigned int x, unsigned int y, unsigned int width, unsigned int height, WNDPROC wndProc);
	~Window();

	// window handles
	HINSTANCE   hInstance;
	HWND        hwnd;
	const char* windowName;

private:

	// creates the window
	void CreateApplicationWindow(const char* title, int x, int y, int width, int height, WNDPROC wndProc);
};

