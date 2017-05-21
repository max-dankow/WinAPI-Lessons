#include "SettingsWindow.h"
#include <commctrl.h>
#include "resource.h"

void CSettingsWindow::Init(HWND settingsWindow)
{
    // Слайдер прозрачности
    SendDlgItemMessage(settingsWindow, IDC_SLIDER_OPACITY, TBM_SETRANGEMIN, FALSE, 0);
    SendDlgItemMessage(settingsWindow, IDC_SLIDER_OPACITY, TBM_SETRANGEMAX, FALSE, 100);
    SendDlgItemMessage(settingsWindow, IDC_SLIDER_OPACITY, TBM_SETPOS, TRUE, modifiedSettings.opacity);
    // Слайдер размера шрифта
    SendDlgItemMessage(settingsWindow, IDC_SLIDER_FONT_SIZE, TBM_SETRANGEMIN, FALSE, 8);
    SendDlgItemMessage(settingsWindow, IDC_SLIDER_FONT_SIZE, TBM_SETRANGEMAX, FALSE, 72);
    SendDlgItemMessage(settingsWindow, IDC_SLIDER_FONT_SIZE, TBM_SETPOS, TRUE, modifiedSettings.fontSize);
    // Checkbox предпросмотр
    previewSettings = false;
    SendDlgItemMessage(settingsWindow, IDC_CHECK_PREVIEW, BM_GETCHECK, BST_UNCHECKED, 0);
}

BOOL CSettingsWindow::SettingsProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    CSettingsWindow* pThis = reinterpret_cast<CSettingsWindow*>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));
    switch (message)
    {
    case WM_INITDIALOG: {
        pThis = reinterpret_cast<CSettingsWindow*>(lParam);
        SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->Init(windowHandle);
        return TRUE;
    }
    case WM_HSCROLL:
        pThis->onScroll(windowHandle, reinterpret_cast<HWND>(lParam));
        break;
    case WM_COMMAND:
        return pThis->onCommand(windowHandle, wParam, lParam);
    }
    return 0;
}


void CSettingsWindow::onScroll(HWND settingsWindow, HWND scrolledItem)
{
    if (scrolledItem == GetDlgItem(settingsWindow, IDC_SLIDER_OPACITY)) {
        modifiedSettings.opacity = SendDlgItemMessage(settingsWindow, IDC_SLIDER_OPACITY, TBM_GETPOS, 0, 0);
        if (previewSettings) {
            editorWindow.SetOpacity(modifiedSettings.opacity);
        }
    }
    if (scrolledItem == GetDlgItem(settingsWindow, IDC_SLIDER_FONT_SIZE)) {
        modifiedSettings.fontSize = SendDlgItemMessage(settingsWindow, IDC_SLIDER_FONT_SIZE, TBM_GETPOS, 0, 0);
        if (previewSettings) {
            editorWindow.SetFontSize(modifiedSettings.fontSize);
        }
    }
}

BOOL CSettingsWindow::onCommand(HWND windowHandle, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case IDC_CHECK_PREVIEW:
    {
        onCheckPreviewMessage(windowHandle, wParam);
        break;
    }
    case IDC_BUTTON_COLOR_FONT:
        onButtonFontColorMessage(windowHandle, wParam);
        break;
    case IDC_BUTTON_BACKGROUND_COLOR:
        onButtonBackgroundColorMessage(windowHandle, wParam);
        break;
    case IDOK:
        finishWith(modifiedSettings, windowHandle, wParam);
        return TRUE;
    case IDCANCEL:
        finishWith(initialSettings, windowHandle, wParam);
        return TRUE;
    }
    return 0;
}

void CSettingsWindow::onCheckPreviewMessage(HWND hwndDlg, WPARAM wParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
        previewSettings = SendDlgItemMessage(hwndDlg, IDC_CHECK_PREVIEW, BM_GETCHECK, 0, 0);
        if (previewSettings) {
            editorWindow.ApplySettings(modifiedSettings);
        }
        break;
    }
}

void CSettingsWindow::onButtonFontColorMessage(HWND hwndDlg, WPARAM wParam)
{
    switch (HIWORD(wParam)) {
    case BN_CLICKED:
        modifiedSettings.fontColor = chooseColor(initialSettings.fontColor, hwndDlg);
        if (previewSettings) {
            editorWindow.SetFontColor(modifiedSettings.fontColor);
        }
        break;
    }
}

void CSettingsWindow::onButtonBackgroundColorMessage(HWND hwndDlg, WPARAM wParam)
{
    switch (HIWORD(wParam)) {
    case BN_CLICKED:
        modifiedSettings.backgroundColor = chooseColor(initialSettings.backgroundColor, hwndDlg);
        if (previewSettings) {
            editorWindow.SetBackgroundColor(modifiedSettings.backgroundColor);
        }
        break;
    }
}

void CSettingsWindow::finishWith(const Settings & settings, HWND windowHandle, WPARAM wParam)
{
    editorWindow.ApplySettings(settings);
    EndDialog(windowHandle, wParam);
}


COLORREF CSettingsWindow::chooseColor(COLORREF initialColor, HWND settingsWindow)
{
    CHOOSECOLORW ccw;
    COLORREF ccref[16];
    memset(&(ccw), 0, sizeof(ccw));
    ccw.lStructSize = sizeof(ccw);
    ccw.hwndOwner = settingsWindow;
    ccw.rgbResult = initialColor;
    ccw.lpCustColors = ccref;
    ccw.Flags = CC_RGBINIT;
    if (ChooseColor(&ccw) != 0) {
        return ccw.rgbResult;
    }
    return initialColor;
}