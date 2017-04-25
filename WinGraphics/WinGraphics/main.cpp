#include <windows.h> 
#include <iostream>
#include <string>
#include <exception>

#include "OverlappedWindow.h"
#include "Utils.h"

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	try {
        COverlappedWindow::RegisterClass();
        COverlappedWindow mainWindow(nCmdShow);
		mainWindow.Create();
		mainWindow.Show(nCmdShow);

		BOOL fGotMessage;
		MSG msg;
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
		return 1;
	}
}