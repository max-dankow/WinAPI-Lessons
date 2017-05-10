#include "TextEditorWindow.h"
#include <memory>
#include <assert.h>
#include "Utils.h"

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
        WS_OVERLAPPEDWINDOW,  // top-level window
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
    SetWindowText(windowHandle, title.c_str());

    createEditControl();
}

void CTextEditorWindow::createEditControl()
{
    assert(windowHandle != NULL);
    editControl = CreateWindowEx(
        0, L"EDIT",   // predefined class 
        NULL,         // no window title 
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
        0, 0, 0, 0,   // set size in WM_SIZE message 
        windowHandle,         // parent window 
        (HMENU) TextEditControlId,   // edit control ID 
        (HINSTANCE)GetWindowLong(windowHandle, GWL_HINSTANCE),
        NULL);

    if (editControl == NULL) {
        throw std::runtime_error("Fail to Create Edit " + std::to_string(GetLastError()));
    }

    // Add text to the window. 
    TCHAR lpszLatin[] = L"Lorem ipsum dolor sit amet, consectetur ";
    SendMessage(editControl, WM_SETTEXT, 0, (LPARAM)lpszLatin);
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
            switch (wmId)
            {
            case TextEditControlId:
                pThis->onMessageFromEdit(HIWORD(wParam));
                break;
            case IDM_EXIT:
                DestroyWindow(windowHandle);
                break;
            default:
                return DefWindowProc(windowHandle, message, wParam, lParam);
            }
        }
        break;
        case WM_SIZE:
            pThis->onResize(LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_CLOSE:
            pThis->onClose();
            break;
        case WM_DESTROY:
            pThis->onDestroy();
            break;
        default:
            return DefWindowProc(windowHandle, message, wParam, lParam);
    }
    return 0;
}

void CTextEditorWindow::onResize(int width, int height)
{
    // Растягиваем EDIT на все окно
    MoveWindow(editControl, 0, 0, width, height, TRUE);
}


std::wstring CTextEditorWindow::getEditText()
{
    auto textLength = GetWindowTextLength(editControl);
    std::unique_ptr<wchar_t[]> buffer(new wchar_t[textLength + 1]);
    GetWindowTextW(editControl, buffer.get(), textLength + 1);
    return std::wstring(buffer.get());
}

void CTextEditorWindow::writeToFile(const std::wstring& content, const std::wstring& path)
{
    CHandle fileHandle(CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));

    if (fileHandle.Get() == INVALID_HANDLE_VALUE) {
        throw std::invalid_argument("Failed to save file");
    }
    size_t bufferSize = content.length() * sizeof(wchar_t);
    if (WriteFile(fileHandle.Get(), content.c_str(), bufferSize, NULL, NULL) == 0) {
        throw std::invalid_argument("Failed to save file");
    }
}

void CTextEditorWindow::save()
{
    std::unique_ptr<wchar_t[]> path(new wchar_t[MAX_PATH + 1]);
    path.get()[0] = L'\0';

    OPENFILENAME ofs;
    memset(&(ofs), 0, sizeof(ofs));
    ofs.lStructSize = sizeof(ofs);
    ofs.hwndOwner = windowHandle;
    ofs.lpstrFile = path.get();
    ofs.nMaxFile = MAX_PATH;
    ofs.lpstrFilter = TEXT("Text (*.txt)\0*.txt\0");
    ofs.lpstrTitle = TEXT("Save File As");
    ofs.Flags = OFN_HIDEREADONLY;
    ofs.lpstrDefExt = TEXT("txt");

    if (GetSaveFileName(&ofs))
    {
        writeToFile(getEditText(), std::wstring(ofs.lpstrFile));
    }
}


void CTextEditorWindow::onClose()
{
    if (isChanged) {
        int result = MessageBox(windowHandle, TEXT("Do you want to save changes?"), TEXT("Save changes"), MB_YESNOCANCEL | MB_ICONWARNING);
        if (result == IDYES || result == IDNO) {
            if (result == IDYES) {
                save();
            }
        } else {
            return;
        }
    }
    onDestroy();
}

void CTextEditorWindow::onDestroy()
{
    PostQuitMessage(0);
}

void CTextEditorWindow::onTextChanged()
{
    isChanged = true;
    SetWindowText(windowHandle, (title + L"*").c_str());
}

void CTextEditorWindow::onMessageFromEdit(int message)
{
    switch(message) {
        case EN_CHANGE:
            onTextChanged();
            break;
    }
}
