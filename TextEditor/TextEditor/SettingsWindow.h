#pragma once
#include <windows.h>
#include "TextEditorWindow.h"

class CSettingsWindow
{
public:
    CSettingsWindow(const Settings& initialSettings , CTextEditorWindow& editorWindow) : 
        previewSettings(false), 
        initialSettings(initialSettings),
        modifiedSettings(initialSettings),
        editorWindow(editorWindow) {}

    INT_PTR ShowSettingsDialog(HWND windowOwner) {
        return DialogBoxParam(GetModuleHandle(NULL),
            MAKEINTRESOURCE(IDD_SETTINGS),
            windowOwner,
            (DLGPROC)SettingsProc,
            reinterpret_cast<LPARAM>(this));
    }
private:
    static BOOL CALLBACK SettingsProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
    void Init(HWND settingsWindow);
    void onScroll(HWND settingsWindow, HWND scrolledItem);
    COLORREF chooseColor(COLORREF initialColor, HWND settingsWindow);

    bool previewSettings;
    Settings initialSettings, modifiedSettings;
    CTextEditorWindow& editorWindow;
};

