#pragma once
#include <windows.h>
#include <string>
#include <process.h>

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
    static unsigned int __stdcall threadProc(void* argument) {
        CVideoCaptureWindow* pThis = static_cast<CVideoCaptureWindow*>(argument);
        __int64 qwDueTime = -00000000LL;
        LARGE_INTEGER   liDueTime;
        // Copy the relative time into a LARGE_INTEGER.
        liDueTime.LowPart = (DWORD)(qwDueTime & 0xFFFFFFFF);
        liDueTime.HighPart = (LONG)(qwDueTime >> 32);
        //SetTimer(NULL, (UINT_PTR)argument, 1000, &MyTimerProc);
        auto timer = CreateWaitableTimer(NULL, FALSE, NULL);
        if (!SetWaitableTimer(timer, &liDueTime, 50, &funTimerAPC_pulse, pThis, FALSE)) {
            ShowError(ErrorMessage(L"SetWaitableTimer", GetLastError()));
        }
        while (true) {
            SleepEx(INFINITE, TRUE);
        }
        return 0;
    }

    static void CALLBACK funTimerAPC_pulse(void* pvArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
    {
        //ShowError(L"Hello from other thread " + std::to_wstring(GetCurrentThreadId()));
        CVideoCaptureWindow* pThis = static_cast<CVideoCaptureWindow*>(pvArg);
        pThis->OnTimer();
    }

    static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
};

