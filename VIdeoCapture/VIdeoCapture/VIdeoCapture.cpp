#include <windows.h>
#include <string>

#include "Resource.h"
#include "Utils.h"
#include "VideoCaptureWindow.h"

#define MAX_LOADSTRING 100

// Глобальные переменные:
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR lpCmdLine,
                     _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    CVideoCaptureWindow videoWindow;

    CoInitialize(NULL);
    try {
        CVideoCaptureWindow::RegisterClass();
        videoWindow.Create(NULL);
        videoWindow.Show(nCmdShow);
        videoWindow.StartPreview();

        MSG msg;
        // Цикл основного сообщения:
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return (int)msg.wParam;
    } catch (std::wstring errorMessage) {
        ShowError(errorMessage);
        return 1;
    }

}