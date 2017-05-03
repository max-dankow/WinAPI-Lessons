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
    void ObtainCurrentImage() {
        previousImage.Release();
        previousImage = std::move(currentImage);
        currentImage = videoCaptureService.ObtainCurrentImage();
        InvalidateRect(windowHandle, &staticImageRect, FALSE);
        UpdateWindow(windowHandle);
    }

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
    CBitmap currentImage, previousImage;
    CVideoCaptureService videoCaptureService;
    RECT staticImageRect = { 400, 0, 800, 300 };

    void dispayDIBitmap(HDC hDc, BITMAPINFOHEADER *pDIBImage);

    static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
};

