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
        switch (LOWORD(wParam))
        {
        case IDC_CHECK_PREVIEW:
        {
            switch (HIWORD(wParam))
            {
            case BN_CLICKED:
                pThis->previewSettings = SendDlgItemMessage(windowHandle, IDC_CHECK_PREVIEW, BM_GETCHECK, 0, 0);
                if (pThis->previewSettings) {
                    pThis->editorWindow.ApplySettings(pThis->modifiedSettings);
                }
                break;
            }
            break;
        }
        case IDC_BUTTON_COLOR_FONT:
            switch (HIWORD(wParam)) {
            case BN_CLICKED:
                pThis->modifiedSettings.fontColor = pThis->chooseColor(pThis->initialSettings.fontColor, windowHandle);
                if (pThis->previewSettings) {
                    pThis->editorWindow.SetFontColor(pThis->modifiedSettings.fontColor);
                }
                break;
            }
            break;
        case IDC_BUTTON_BACKGROUND_COLOR:
            switch (HIWORD(wParam)) {
            case BN_CLICKED:
                pThis->modifiedSettings.backgroundColor = pThis->chooseColor(pThis->initialSettings.backgroundColor, windowHandle);
                if (pThis->previewSettings) {
                    pThis->editorWindow.SetBackgroundColor(pThis->modifiedSettings.backgroundColor);
                }
                break;
            }
            break;
        case IDOK:
            pThis->editorWindow.ApplySettings(pThis->modifiedSettings);
            EndDialog(windowHandle, wParam);
            return TRUE;
        case IDCANCEL:
            pThis->editorWindow.ApplySettings(pThis->initialSettings);
            EndDialog(windowHandle, wParam);
            return TRUE;
        }
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