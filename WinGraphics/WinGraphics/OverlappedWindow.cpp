#include "OverlappedWindow.h"
#include "Utils.h"
#include <assert.h>

void COverlappedWindow::RegisterClass()
{
    WNDCLASSEX wcx;
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = windowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = GetModuleHandle(NULL);
    wcx.hIcon = NULL;
    wcx.hCursor = NULL;
    wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = ClassName;
    wcx.hIconSm = NULL;
    if (RegisterClassExW(&wcx) == 0) {
        throw std::runtime_error("Fail to RegisterClass " + std::to_string(GetLastError()));
    }
}

void COverlappedWindow::Create()
{
    windowHandle = CreateWindow(
        ClassName,  // name of window class
        title.c_str(),  // title-bar string
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,  // top-level window
        CW_USEDEFAULT,  // default horizontal position
        CW_USEDEFAULT,  // default vertical position
        CW_USEDEFAULT,  // default width
        CW_USEDEFAULT,  // default height
        (HWND)NULL,  // no owner window
        (HMENU)NULL,  // use class menu
        GetModuleHandle(NULL),  // handle to current application instance
        this);  // ѕередаетс€ чтобы использовать нестатические члены COverlappedWindow в статической windowProc

    if (windowHandle == NULL) {
        throw std::runtime_error("Fail to CreateWindow " + std::to_string(GetLastError()));
    }
}

void COverlappedWindow::Show(int cmdShow) const
{
    // Show the window and send a WM_PAINT message to the window procedure. 
    ShowWindow(windowHandle, cmdShow);
    UpdateWindow(windowHandle);
}

LRESULT COverlappedWindow::windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    // ¬осстанавливаем указатель this на COverlappedWindow
    COverlappedWindow* pThis = reinterpret_cast<COverlappedWindow*>(GetWindowLongPtr(window, GWLP_USERDATA));

    switch (message) {
    case WM_NCCREATE:
    {
        // ѕри создании окна, в параметрах сообщени€ WM_NCCREATE будет this
        // ¬ этом случае, его нужно запомнить в параметрах окна.
        pThis = static_cast<COverlappedWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

        // —оздаем дочерние CEllipseWindow
        CEllipseWindow::RegisterClassW();
        int i = 0;
        for (CEllipseWindow& ellipseWindow : pThis->ellipseWindows) {
            ellipseWindow.Create(window);
            ellipseWindow.Show(SW_SHOW);
        }
        return TRUE;
    }
    case WM_SIZE:
        for (int i = 0; i < pThis->ellipseWindows.size(); ++i) {
            pThis->updateEllipseWindow(i);
        }
        return 0;
    case WM_DESTROY:
        SetWindowLongPtr(window, GWLP_USERDATA, NULL);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(window, message, wParam, lParam);
    }
    if (pThis == NULL) {
        ShowError("Fail to get pointer to COverlappedWindow " + std::to_string(GetLastError()));
        PostQuitMessage(1);
    }
    return 0;
}

// TODO: generalise
void COverlappedWindow::updateEllipseWindow(int index)
{
    assert(index >= 0 && index < 4);
    int col = index % 2;
    int row = index / 2;
    HWND childWindow = ellipseWindows[index].GetWindowHandle();

    RECT parentRect;
    GetClientRect(windowHandle, &parentRect);
    SetWindowPos(childWindow,
        HWND_TOP,
        (parentRect.right / 2) * col,
        (parentRect.bottom / 2) * row,
        parentRect.right / 2,
        parentRect.bottom / 2,
        0);
}
