#include "EllipseWindow.h"
#include <exception>
#include <string>

#include "Utils.h"

class CDeviceContext
{
public:
	CDeviceContext(HWND hWnd, LPPAINTSTRUCT paintStruct) : window(hWnd), paintStruct(paintStruct) {
		context = BeginPaint(hWnd, paintStruct);
	}
	~CDeviceContext() {
		EndPaint(window, paintStruct);
	}

	HDC getContext() const {
		return context;
	}
	// todo: delete copy constructor
private:
	HDC context;
	HWND window;
	LPPAINTSTRUCT paintStruct;
};
// TODO: убрать —-cast
// —тоит заметить, что из-за define RegisterClass RegisterClassW теперь везде происходит замена.
void CEllipseWindow::RegisterClass()
{
	WNDCLASSEX wcx;
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = windowProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = GetModuleHandle(NULL);
	wcx.hIcon = NULL;
	wcx.hCursor = NULL;
	wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = ClassName;
	wcx.hIconSm = NULL;
	if (RegisterClassExW(&wcx) == 0) {
		throw std::runtime_error("Fail to RegisterClass " + std::to_string(GetLastError()));
	}
}

void CEllipseWindow::Create(HWND parentWindow)
{
    auto style = (parentWindow == NULL) ? WS_OVERLAPPEDWINDOW : WS_CHILD;
	windowHandle = CreateWindow(
	ClassName,  // name of window class
	title.c_str(),  // title-bar string
	style,
	CW_USEDEFAULT,  // default horizontal position
	CW_USEDEFAULT,  // default vertical position
	CW_USEDEFAULT,  // default width
	CW_USEDEFAULT,  // default height
	parentWindow,
	(HMENU)NULL,  // use class menu
	GetModuleHandle(NULL),  // handle to current application instance
	this);  // ѕередаетс€ чтобы использовать нестатические члены COverlappedWindow в статической windowProc

	if (windowHandle == NULL) {
		throw std::runtime_error("Fail to CreateWindow " + std::to_string(GetLastError()));
	}
}

void CEllipseWindow::Show(int cmdShow) const
{
	// Show the window and send a WM_PAINT message to the window procedure. 
	ShowWindow(windowHandle, cmdShow);
	UpdateWindow(windowHandle);
	StartTimer();
}

void CEllipseWindow::StartTimer() const
{
	SetTimer(windowHandle, 1, TimerElapseMs, NULL);
}

void CEllipseWindow::StopTimer() const
{
	KillTimer(windowHandle, 1);
}

LRESULT CALLBACK CEllipseWindow::windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	// ¬осстанавливаем указатель this на CEllipseWindow
	CEllipseWindow* pThis = reinterpret_cast<CEllipseWindow*>(GetWindowLongPtr(handle, GWLP_USERDATA));

	switch (message) {
	case WM_NCCREATE:
		// ѕри создании окна, в параметрах сообщени€ WM_NCCREATE будет this
		// ¬ этом случае, его нужно запомнить в параметрах окна.
		pThis = static_cast<CEllipseWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
		return TRUE;
	case WM_PAINT:
		pThis->OnDraw();
		break;
	case WM_ERASEBKGND:
		return (LRESULT)1; // Say we handled it.
	case WM_TIMER:
		pThis->OnTimer();
		break;
    case WM_LBUTTONDOWN:
        SetFocus(handle);
        return 0;
	case WM_DESTROY:
		SetWindowLongPtr(handle, GWLP_USERDATA, NULL);
		pThis->StopTimer();
		pThis->OnDestroy();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(handle, message, wParam, lParam);
	}
	if (pThis == NULL) {
		ShowError("Fail to get pointer to CEllipseWindow " + std::to_string(GetLastError()));
		PostQuitMessage(1);
	}
	return 0;
}

void CEllipseWindow::OnDraw()
{
	PAINTSTRUCT paintStruct;
	CDeviceContext contextHolder(windowHandle, &paintStruct);
	HDC context = contextHolder.getContext();

	// example from msdn
	RECT rc;
	GetClientRect(windowHandle, &rc);
	HDC hdcMem = CreateCompatibleDC(context);
	HBITMAP hbmMem, hbmOld;
	hbmMem = CreateCompatibleBitmap(context, rc.right - rc.left, rc.bottom - rc.top);

	hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

	HBRUSH hbrBkGnd = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	FillRect(hdcMem, &rc, hbrBkGnd);
	DeleteObject(hbrBkGnd);
	
    Rectangle(hdcMem, rc.left, rc.top, rc.right, rc.bottom);
	ellipse.draw(hdcMem, isFocused());

	BitBlt(context,
		rc.left, rc.top,
		rc.right - rc.left, rc.bottom - rc.top,
		hdcMem,
		0, 0,
		SRCCOPY);

	SelectObject(hdcMem, hbmOld);
	DeleteObject(hbmMem);
	DeleteDC(hdcMem);
}

void CEllipseWindow::OnTimer()
{
	RECT rect;
	GetClientRect(windowHandle, &rect);
    if (isFocused()) {
        wasActive = true;
        ellipse.move(rect);
        InvalidateRect(windowHandle, NULL, TRUE);
    } else {
        wasActive = false;
        InvalidateRect(windowHandle, NULL, TRUE);
    }
}
