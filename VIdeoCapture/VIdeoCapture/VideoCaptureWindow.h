#pragma once
#include <windows.h>
#include <string>

#include "VideoCaptureService.h"

class CVideoCaptureWindow
{
public:
    CVideoCaptureWindow(const std::wstring title = L"Video Capture") : title(title) { }

    static void RegisterClass();
    void Create(HWND parentWindow = NULL);
    void Show(int cmdShow) const;
    void StartPreview();

    HWND GetWindowHandle() const {
        return windowHandle;
    }

protected:
    void OnDestroy();
    void OnDraw();
    void OnTimer();
private:
    static constexpr wchar_t* ClassName = L"CVideoCaptureWindow";

    std::wstring title;
    HWND windowHandle;
    CVideoCaptureService videoCaptureService;

    static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
};

