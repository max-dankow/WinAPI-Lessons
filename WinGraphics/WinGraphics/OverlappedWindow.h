#pragma once
#include <Windows.h>
#include <string>
#include <array>

#include "EllipseWindow.h"

class COverlappedWindow
{
public:
    COverlappedWindow(int nCmdShow, const std::wstring title = L"Main window") : title(title), nCmdShow(nCmdShow){ }
    static void RegisterClass();
    void Create();
    void Show(int cmdShow) const;

private:
    static constexpr wchar_t* ClassName = L"COverlappedWindow";
    std::wstring title;
    HWND windowHandle;
    std::array<CEllipseWindow, 4> ellipseWindows;
    int nCmdShow;

    static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
    void updateEllipseWindow(int index);
};

