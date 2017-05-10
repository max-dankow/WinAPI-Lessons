#pragma once
#include <windows.h>
#include <string>
#include "resource.h"

class CTextEditorWindow
{
public:
    CTextEditorWindow(const std::wstring title = L"Main window") : title(title) {}

    static void RegisterClass();
    void Create();
    void Show(int cmdShow) const;

    HWND GetWindowHandle() const {
        return windowHandle;
    }

private:
    static constexpr wchar_t* ClassName = L"CTextEditor";
    std::wstring title;
    HWND windowHandle;

    static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
};

