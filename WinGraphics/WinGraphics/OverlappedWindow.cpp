#include "OverlappedWindow.h"
#include "Utils.h"
#include "resource.h"
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
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
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
        static_cast<HWND>(NULL),  // no owner window
        static_cast<HMENU>(NULL),  // use class menu
        GetModuleHandle(NULL),  // handle to current application instance
        this);  // ѕередаетс€ чтобы использовать нестатические члены COverlappedWindow в статической windowProc

    if (windowHandle == NULL) {
        throw std::runtime_error("Fail to CreateWindow " + std::to_string(GetLastError()));
    }
    createEllipseWindows();
}

void COverlappedWindow::createEllipseWindows()
{
    CEllipseWindow::RegisterClassW();
    ellipseWindows.resize(columnsCount * rowsCount);
    for (CEllipseWindow& ellipseWindow : ellipseWindows) {
        ellipseWindow.Create(windowHandle);
        ellipseWindow.Show(SW_SHOW);
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
        case WM_NCCREATE:{
            // ѕри создании окна, в параметрах сообщени€ WM_NCCREATE будет this
            // ¬ этом случае, его нужно запомнить в параметрах окна.
            pThis = static_cast<COverlappedWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
            SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            return TRUE;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case ID_KEY_UP:
                    pThis->moveFocus(0, -1);
                    break;
                case ID_KEY_RIGHT:
                    pThis->moveFocus(1, 0);
                    break;
                case ID_KEY_DOWN:
                    pThis->moveFocus(0, 1);
                    break;
                case ID_KEY_LEFT:
                    pThis->moveFocus(-1, 0);
                    break;
                default:
                    return DefWindowProc(window, message, wParam, lParam);
            }
            break;
        }
        case WM_SIZE:
            pThis->resizeEllipseWindows();
            break;
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

void COverlappedWindow::resizeEllipseWindow(int index)
{
    assert(index >= 0 && index < ellipseWindows.size());
    int col = getColumn(index);
    int row = getRow(index);
    HWND childWindow = ellipseWindows[index].GetWindowHandle();

    RECT parentRect;
    GetClientRect(windowHandle, &parentRect);
    SetWindowPos(childWindow,
        HWND_TOP,
        (parentRect.right / columnsCount) * col,
        (parentRect.bottom / rowsCount) * row,
        parentRect.right / columnsCount,
        parentRect.bottom / rowsCount,
        0);
}

void COverlappedWindow::resizeEllipseWindows()
{
    for (int i = 0; i < ellipseWindows.size(); ++i) {
        resizeEllipseWindow(i);
    }
}

int COverlappedWindow::findActiveEllipseWindow()
{
    auto focusedWindow = GetFocus();
    for (size_t i = 0; i < ellipseWindows.size(); ++i) {
        if (focusedWindow == ellipseWindows[i].GetWindowHandle()) {
            return i;
        }
    }
    return -1;
}

void COverlappedWindow::moveFocus(int dCol, int dRow)
{
    int focusedIndex = findActiveEllipseWindow();
    if (focusedIndex < 0) {
        return;
    }

    int oldCol = getColumn(focusedIndex);
    int oldRow = getRow(focusedIndex);

    int newCol = (oldCol + dCol + columnsCount) % columnsCount;
    int newRow = (oldRow + dRow + rowsCount) % rowsCount;
    int newIndex = getIndex(newCol, newRow);
    assert(newIndex >= 0 && newIndex < ellipseWindows.size());

    SetFocus(ellipseWindows[newIndex].GetWindowHandle());
}
