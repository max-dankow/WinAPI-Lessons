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
    void ObtainCurrentImage();

    HWND GetWindowHandle() const {
        return windowHandle;
    }

protected:
    void OnDestroy();
    void OnDraw();

    // Таймер дополнительного рабочего потока обработки изображения
    void OnTimer();
private:
    static constexpr wchar_t* ClassName = L"CVideoCaptureWindow";
    static const LONG timerPeriod = 10;

    std::wstring title;
    HWND windowHandle;
    CBitmap currentImage, previousImage;
    CVideoCaptureService videoCaptureService;
    const RECT PreviewRect = { 0, 0, 400, 300 };
    const RECT PreviousImageRect = { 400, 0, 800, 300 };
    const RECT CurrentImageRect = { 800, 0, 1200, 300 };
    const unsigned WindowHeight = 400;
    const unsigned WindowWidth = 800;

    // Все для дополнительного рабочего потока
    CHandle worker;
    std::mutex mutex;
    CHandle terminationEvent;
    CHandle timer;

    void createTerminationEvent();
    void detectMotion();
    void stopMotionDetection();

    // Функция рабочего потока
    static unsigned int __stdcall threadProc(void* argument);

    // Функция обработки изображения, вызываемая по таймеру
    static void CALLBACK timerProc(void* pvArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue);

    // Оконная процедура, обработка сообшений
    static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
};

