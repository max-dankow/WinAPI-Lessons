#pragma once
#include <Windows.h>
#include <string>

// Переключает GDI объект, при разрушении устанавливает изначальный объект
class CObjectSwitcher {
public:
    CObjectSwitcher(HDC context, HGDIOBJ newObject) : context(context) {
        previous = SelectObject(context, newObject);
    }

    CObjectSwitcher(const CObjectSwitcher&) = delete;
    CObjectSwitcher operator=(const CObjectSwitcher&) = delete;

    ~CObjectSwitcher() {
        SelectObject(context, previous);
    }
private:
    HDC context;
    HGDIOBJ previous;
};

class CEllipse {
public:
    CEllipse(int left, int top, int height, int width, int speedX=5, int speedY=5) : 
        left(left), top(top), 
        height(height), width(width),
        speedX(speedX), speedY(speedY),
        brushActive(CreateSolidBrush(DefaultBrushColorActive)),
        brushNotActive(CreateSolidBrush(DefaultBrushColorNotActive)) { }

    ~CEllipse() {
        DeleteObject(brushActive);
    }

    void Draw(HDC context, bool isActive) const {
        CObjectSwitcher penSwitcher(context, GetStockObject(NULL_PEN));
        auto actualBrush = (isActive) ? brushActive : brushNotActive;
        CObjectSwitcher brushSwitcher(context, actualBrush);
        Ellipse(context, left, top, left + width, top + height);
    }

    void Move(RECT border) {
        int bottom = top + height;
        if (top + speedY < border.top || bottom + speedY > border.bottom) {
            speedY *= -1;
        }
        int right = left + width;
        if (left + speedX < border.left || right + speedX > border.right) {
            speedX *= -1;
        }
        top += speedY;
        left += speedX;
    }

private:
    static const COLORREF DefaultBrushColorActive = RGB(128, 0, 128);
    static const COLORREF DefaultBrushColorNotActive = RGB(128, 128, 128);
    int left, top, height, width;
    int speedX, speedY;
    HBRUSH brushActive, brushNotActive;
};

class CEllipseWindow
{
public:
    CEllipseWindow(const std::wstring title = L"Ellipse") : title(title), ellipse(0, 0, 50, 100), wasActive(false) { }

    static void RegisterClass();
    void Create(HWND parentWindow = NULL);
    void Show(int cmdShow) const;

    HWND GetWindowHandle() const {
        return windowHandle;
    }

protected:
    void OnDestroy();
    void OnDraw();
    void OnTimer();
private:
    static constexpr wchar_t* ClassName = L"CEllipseWindow";

    CEllipse ellipse;
    std::wstring title;
    HWND windowHandle;
    bool wasActive;

    static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

    bool isFocused() {
        HWND focusedWindow = GetFocus();
        return focusedWindow == windowHandle;
    }
};
