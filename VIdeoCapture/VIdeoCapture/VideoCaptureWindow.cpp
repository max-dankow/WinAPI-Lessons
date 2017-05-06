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
    auto style = (parentWindow == NULL) ? WS_OVERLAPPEDWINDOW : WS_CHILD;
    windowHandle = CreateWindow(
        ClassName,  // name of window class
        title.c_str(),  // title-bar string
        style | WS_BORDER,
        0,  // default horizontal position
        0,  // default vertical position
        800,  // default width
        300,  // default height
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
    }
    catch (std::wstring errorMessage) {
        ShowError(errorMessage);
    }
}

void CVideoCaptureWindow::dispayImage(HDC hDC, RECT imageRect, CBitmap& image)
{
    
    //auto hBitmap = image.GetBitmap(hDC);

    // TODO: remove - Закомментированные действия перенесены в CBitmap
    //HDC hMemDC = CreateCompatibleDC(hDC);
    //hBitmap = reinterpret_cast<HBITMAP>(SelectObject(hMemDC, hBitmap));
    //BitBlt(hDC, imageRect.left, imageRect.top, image.GetWidth(), image.GetHeight(), hMemDC, 0, 0, SRCCOPY);
    //DeleteObject(SelectObject(hMemDC, hBitmap));
    //DeleteDC(hMemDC);
}

void CVideoCaptureWindow::detectMotion()
{
    if (!previousImage.IsNull() && !currentImage.IsNull()) {
        CMotionDetector::Detect(previousImage, currentImage);
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
    case WM_CREATE:
        SetTimer(windowHandle, 1, 500, NULL);
        break;
    case WM_TIMER:
        pThis->ObtainCurrentImage();
        pThis->detectMotion();
        InvalidateRect(windowHandle, &pThis->previousImageRect, FALSE);
        InvalidateRect(windowHandle, &pThis->currentImageRect, FALSE);
        UpdateWindow(windowHandle);
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        //pThis->OnDraw();
        PAINTSTRUCT paintStruct;
        CDeviceContext contextHolder(windowHandle, &paintStruct);
        HDC context = contextHolder.getContext();
        if (!pThis->currentImage.IsNull()) {
            pThis->currentImage.Show(context, pThis->currentImageRect);
            //pThis->dispayImage(context, pThis->currentImageRect, pThis->currentImage.GetImage());
        }
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