#pragma once
#include <Windows.h>
#include <string>
#include <vector>

#include "EllipseWindow.h"

class COverlappedWindow
{
public:
    COverlappedWindow(size_t columnsCount = 2, size_t rowsCount = 2, const std::wstring title = L"Main window") : 
        columnsCount(columnsCount), rowsCount(rowsCount), title(title) { }

    static void RegisterClass();
    void Create();
    void Show(int cmdShow) const;

    HWND GetWindowHandle() const {
        return windowHandle;
    }

private:
    static constexpr wchar_t* ClassName = L"COverlappedWindow";
    std::wstring title;
    HWND windowHandle;
    size_t columnsCount, rowsCount;
    std::vector<CEllipseWindow> ellipseWindows;

    static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
    void createEllipseWindows();
    void resizeEllipseWindow(int index);
    void resizeEllipseWindows();
    int findActiveEllipseWindow();
    void moveFocus(int dCol, int dRow);

    size_t getColumn(size_t index) {
        return index % columnsCount;
    }

    size_t getRow(size_t index) {
        return index / columnsCount;
    }

    size_t getIndex(size_t col, size_t row) {
        return row * columnsCount + col;
    }
};
