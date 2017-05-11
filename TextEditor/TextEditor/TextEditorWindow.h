#pragma once
#include <windows.h>
#include <string>
#include "resource.h"
#include "Utils.h"

struct Settings {
    int opacity;
    int fontSize;
    COLORREF fontColor, backgroundColor;
};

class CTextEditorWindow
{
public:
    CTextEditorWindow(const std::wstring title = L"Main window") :
        title(title), 
        isChanged(false),
        settings({100, 17, RGB(0, 0, 0), RGB(255, 255, 255) }) {}

    static void RegisterClass();
    void Create();
    void Show(int cmdShow) const;

    HWND GetWindowHandle() const {
        return windowHandle;
    }

    void SetFontColor(COLORREF newColor) {
        settings.fontColor = newColor;
        redraw();
    }
    void SetBackgroundColor(COLORREF newColor) {
        settings.backgroundColor = newColor;
        redraw();
    }
    void SetOpacity(int opacity);
    void SetFontSize(int fontSize);
    void ApplySettings(const Settings& settings);

private:
    static constexpr wchar_t* ClassName = L"CTextEditor";
    static const int TextEditControlId = 0;
    std::wstring title;
    HWND windowHandle;
    HWND editControl;
    bool isChanged;
    Settings settings;
    CGdiObject<HFONT> font;

    static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
    void redraw() const;
    void createEditControl();
    std::wstring getEditText();
    void writeToFile(const std::wstring& content, const std::wstring& path);
    void save();
    
    void onResize(int width, int height);
    void onClose();
    void onDestroy();
    void onTextChanged();
    void onMessageFromEdit(int message);
    void onOpenSettings();
    

};

