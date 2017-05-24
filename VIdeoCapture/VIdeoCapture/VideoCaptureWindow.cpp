#include "VideoCaptureWindow.h"
#include "Utils.h"
#include "MotionDetector.h"

void CVideoCaptureWindow::RegisterClass()
{
    WNDCLASSEX wcx;
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = windowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = GetModuleHandle(NULL);
    wcx.hIcon = NULL;
    wcx.hCursor = LoadCursor(NULL, IDC_HAND);
    wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = ClassName;
    wcx.hIconSm = NULL;
    if (RegisterClassExW(&wcx) == 0) {
        throw std::runtime_error("Fail to RegisterClass " + std::to_string(GetLastError()));
    }
}

void CVideoCaptureWindow::Create(HWND parentWindow)
{
    // TODO: no constatns
    auto style = (parentWindow == NULL) ? WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX : WS_CHILD;
    windowHandle = CreateWindow(
        ClassName,  // name of window class
        title.c_str(),  // title-bar string
        style,
        0,  // default horizontal position
        0,  // default vertical position
        WindowWidth,  // default width
        WindowHeight,  // default height
        parentWindow,
        static_cast<HMENU>(NULL),
        GetModuleHandle(NULL),  // handle to current application instance
        this);

    if (windowHandle == NULL) {
        throw std::runtime_error("Fail to CreateWindow " + std::to_string(GetLastError()));
    }

    try {
        videoCaptureService.Init(windowHandle);
    }
    catch (std::wstring errorMessage) {
        ShowError(errorMessage);
    }
}

void CVideoCaptureWindow::Show(int cmdShow) const
{
    // Show the window and send a WM_PAINT message to the window procedure. 
    ShowWindow(windowHandle, cmdShow);
    UpdateWindow(windowHandle);
}

void CVideoCaptureWindow::StartPreview()
{
    try {
        videoCaptureService.StartPreview(previewRect);
        _beginthreadex(NULL, 0, &CVideoCaptureWindow::threadProc, this, 0, NULL);
    }
    catch (std::wstring errorMessage) {
        ShowError(errorMessage);
    }
}

void CVideoCaptureWindow::OnTimer()
{
    ObtainCurrentImage();
    detectMotion();
    InvalidateRect(windowHandle, &previousImageRect, FALSE);
    InvalidateRect(windowHandle, &currentImageRect, FALSE);
    UpdateWindow(windowHandle);
}

void CVideoCaptureWindow::detectMotion()
{
    if (!previousImage.IsNull() && !currentImage.IsNull()) {
        std::lock_guard<std::mutex> lock(mutex);
        auto moveMask = CMotionDetector::Detect(previousImage, currentImage);
        CMotionDetector::saveMoveMaskToBitmap(moveMask, previousImage);
    }
}

LRESULT CALLBACK CVideoCaptureWindow::windowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Получаем указатель на объект соответствующего класса
    CVideoCaptureWindow* pThis = reinterpret_cast<CVideoCaptureWindow*>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));

    switch (message) {
    case WM_NCCREATE:
        // Запоминаем указатель на объект соответствующего класса при запуске
        pThis = static_cast<CVideoCaptureWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
        SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));


        return TRUE;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        //pThis->OnDraw();
        PAINTSTRUCT paintStruct;
        CDeviceContext contextHolder(windowHandle, &paintStruct);
        HDC context = contextHolder.getContext();
        //if (!pThis->currentImage.IsNull()) {
        //    pThis->currentImage.Show(context, pThis->currentImageRect);
        //    //pThis->dispayImage(context, pThis->currentImageRect, pThis->currentImage.GetImage());
        //}
        std::lock_guard<std::mutex> lock(pThis->mutex);
        if (!pThis->previousImage.IsNull()) {
            pThis->previousImage.Show(context, pThis->previousImageRect);

            //pThis->dispayImage(context, pThis->previousImageRect, pThis->previousImage.GetImage());
        }
        break;
    }
    case WM_DESTROY:
        //pThis->OnDestroy();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(windowHandle, message, wParam, lParam);
    }

    if (pThis == NULL) {
        ShowError(L"Fail to get pointer to window class " + std::to_wstring(GetLastError()));
        PostQuitMessage(1);
    }

    return 0;
}