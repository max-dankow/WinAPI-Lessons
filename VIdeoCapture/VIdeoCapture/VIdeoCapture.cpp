#include <windows.h>

#include <Dshow.h>
#include <objbase.h>

#include <string>
#include <exception>

#include "Resource.h"
#include "Utils.h"
#include "VideoCaptureWindow.h"

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VIDEOCAPTURE, szWindowClass, MAX_LOADSTRING);
    //MyRegisterClass(hInstance);
    CVideoCaptureWindow videoWindow;

    // Выполнить инициализацию приложения:
    //auto mainWindow = InitInstance(hInstance, nCmdShow, &videoWindow);
    //if (!mainWindow) {
    //    return 1;
    //}
    CoInitialize(NULL);
    try {
        CVideoCaptureWindow::RegisterClass();
        videoWindow.Create(NULL);
        videoWindow.Show(nCmdShow);
        videoWindow.StartPreview();
    }
    catch (std::wstring errorMessage) {
        ShowError(errorMessage);
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VIDEOCAPTURE));
    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}