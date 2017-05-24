#pragma once
#include <windows.h>
#include <string>
#include <process.h>
#include <mutex>

#include "VideoCaptureService.h"
#include "Bitmap.h"
#include "Handle.h"

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
    const RECT previewRect = { 0, 0, 400, 300 };
    const RECT previousImageRect = { 400, 0, 800, 300 };
    const RECT currentImageRect = { 800, 0, 1200, 300 };
    const unsigned WindowHeight = 600;
    const unsigned WindowWidth = 800;

    // Синхронихация с рабочим потоком
    std::mutex mutex;
    CHandle terminationEvent;
    CHandle worker;

    CHandle timer;

    void CreateTerminationEvent();
    void detectMotion();
    void stopMotionDetection();
    static unsigned int __stdcall threadProc(void* argument) {
        CVideoCaptureWindow* pThis = static_cast<CVideoCaptureWindow*>(argument);
        __int64 qwDueTime = -00000000LL;
        LARGE_INTEGER   liDueTime;
        // Copy the relative time into a LARGE_INTEGER.
        liDueTime.LowPart = (DWORD)(qwDueTime & 0xFFFFFFFF);
        liDueTime.HighPart = (LONG)(qwDueTime >> 32);
        //SetTimer(NULL, (UINT_PTR)argument, 1000, &MyTimerProc);
        pThis->timer = CreateWaitableTimer(NULL, FALSE, NULL);
        if (!SetWaitableTimer(pThis->timer.Get(), &liDueTime, 10, &funTimerAPC_pulse, pThis, FALSE)) {
            ShowError(ErrorMessage(L"SetWaitableTimer", GetLastError()));
        }
        // Когда произойдет сигнал о завершении, таймер будет отключен и alertable функция вернет упревление 
        // и сможет выйти из цикла.
        while (WaitForSingleObject(pThis->terminationEvent.Get(), 0) != WAIT_OBJECT_0) {
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

