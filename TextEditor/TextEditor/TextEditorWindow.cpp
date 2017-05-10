#include "TextEditorWindow.h"

void CTextEditorWindow::RegisterClass()
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
    wcx.lpszMenuName = MAKEINTRESOURCEW(IDC_TEXTEDITOR);
    wcx.lpszClassName = ClassName;
    wcx.hIconSm = NULL;
    if (RegisterClassExW(&wcx) == 0) {
        throw std::runtime_error("Fail to RegisterClass " + std::to_string(GetLastError()));
    }
}

void CTextEditorWindow::Create()
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
        this);

    if (windowHandle == NULL) {
        throw std::runtime_error("Fail to CreateWindow " + std::to_string(GetLastError()));
    }
}

void CTextEditorWindow::Show(int cmdShow) const
{
    ShowWindow(windowHandle, cmdShow);
    UpdateWindow(windowHandle);
}

LRESULT CTextEditorWindow::windowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Получение указателя на с++ объект окна
    CTextEditorWindow* pThis = reinterpret_cast<CTextEditorWindow*>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));
    switch (message) {
        case WM_NCCREATE: {
            pThis = static_cast<CTextEditorWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
            SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            return TRUE;
        }
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(windowHandle);
                break;
            default:
                return DefWindowProc(windowHandle, message, wParam, lParam);
            }
        }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(windowHandle, message, wParam, lParam);
    }
    return 0;
}
