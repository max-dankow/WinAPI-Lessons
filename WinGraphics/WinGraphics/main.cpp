#include <windows.h> 
#include <iostream>
#include <string>
#include <exception>

#include "OverlappedWindow.h"

// Global variable 

HINSTANCE hinst;

// Function prototypes. 

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

void ShowError(const std::string &message)
{
	MessageBoxA(0, message.c_str(), "Error", MB_OK);
}

// Application entry point.

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	try {
		COverlappedWindow::RegisterClass();
		COverlappedWindow mainWindow;
		mainWindow.Create();
		mainWindow.Show(nCmdShow);

		BOOL fGotMessage;
		while ((fGotMessage = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0 && fGotMessage != -1)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return msg.wParam;
		UNREFERENCED_PARAMETER(lpCmdLine);
	}
	catch (std::runtime_error e) {
		ShowError(e.what());
	}
}