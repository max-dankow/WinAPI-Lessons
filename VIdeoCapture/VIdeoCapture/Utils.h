#pragma once
#include <windows.h>
#include <dshow.h>
#include <string>


class CDeviceContext
{
public:
    CDeviceContext(HWND hWnd, LPPAINTSTRUCT paintStruct) : window(hWnd), paintStruct(paintStruct) {
        context = BeginPaint(hWnd, paintStruct);
    }

    CDeviceContext(const CDeviceContext&) = delete;
    CDeviceContext operator=(const CDeviceContext&) = delete;

    ~CDeviceContext() {
        EndPaint(window, paintStruct);
    }

    HDC getContext() const {
        return context;
    }

private:
    HDC context;
    HWND window;
    LPPAINTSTRUCT paintStruct;
};

void ShowError(const std::wstring &message);

std::wstring ErrorMessage(const std::wstring &message, DWORD errorCode);
std::wstring ErrorMessage(HRESULT hr);

void ThrowIfError(const std::wstring tag, HRESULT result);