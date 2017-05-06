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
        currentImage = CBitmap(GetDC(windowHandle), videoCaptureService.ObtainCurrentImage());
        InvalidateRect(windowHandle, &previousImageRect, FALSE);
        InvalidateRect(windowHandle, &currentImageRect, FALSE);
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
    RECT previewRect = { 0, 0, 400, 300 };
    RECT previousImageRect = { 400, 0, 800, 300 };
    RECT currentImageRect = { 800, 0, 1200, 300 };
    

    void dispayImage(HDC hDc, RECT imageRect, CBitmap& image);
    void detectMotion();

    static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
};

