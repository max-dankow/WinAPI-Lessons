#pragma once
#include <windows.h>
#include <string>
#include "resource.h"

class CTextEditorWindow
{
public:
    CTextEditorWindow(const std::wstring title = L"Main window") :
        title(title), 
        isChanged(false), 
        opacity(100), 
        previewSettings(false),
        fontColor(RGB(0, 0, 0)),
        backgroundColor(RGB(255, 255, 255)){}

    static void RegisterClass();
    void Create();
    void Show(int cmdShow) const;

    HWND GetWindowHandle() const {
        return windowHandle;
    }

private:
    static constexpr wchar_t* ClassName = L"CTextEditor";
    static const int TextEditControlId = 0;
    std::wstring title;
    HWND windowHandle;
    HWND editControl;
    bool isChanged;
    int opacity;
    bool previewSettings;
    COLORREF fontColor, backgroundColor;

    static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK SettingsProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
    void redraw() const;
    void createEditControl();
    std::wstring getEditText();
    void writeToFile(const std::wstring& content, const std::wstring& path);
    void save();
    void setOpacity(int opacity);
    void applySettings(HWND settingsWindow);
    COLORREF chooseColor(COLORREF initialColor, HWND settingsWindow);

    void onResize(int width, int height);
    void onClose();
    void onDestroy();
    void onTextChanged();
    void onMessageFromEdit(int message);
    void onOpenSettings();
    void onSettingsWindowCreated(HWND settingsWindow);
    void onScroll(HWND settingsWindow, HWND scrolledItem);

};

