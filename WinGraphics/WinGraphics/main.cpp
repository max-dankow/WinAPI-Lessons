#include <windows.h>
#include <winuser.h>
#include <iostream>
#include <string>
#include <exception>

#include "OverlappedWindow.h"
#include "Utils.h"
#include "resource.h"

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try {
        COverlappedWindow::RegisterClass();
        COverlappedWindow mainWindow;
        mainWindow.Create();
        mainWindow.Show(nCmdShow);

        HACCEL hAccelTable = LoadAccelerators(hinstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
        BOOL fGotMessage;
        MSG msg;
        while ((fGotMessage = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0 && fGotMessage != -1)
        {
            if (!TranslateAccelerator(mainWindow.GetWindowHandle(), hAccelTable, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        return msg.wParam;
        UNREFERENCED_PARAMETER(lpCmdLine);
    }
    catch (std::runtime_error e) {
        ShowError(e.what());
        return 1;
    }
}