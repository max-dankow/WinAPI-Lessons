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
	wcx.hCursor = LoadCursor(NULL, IDC_HAND);
	wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
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
	    static_cast<HMENU>(NULL),
	    GetModuleHandle(NULL),  // handle to current application instance
	    this);  // ѕередаетс€ чтобы использовать нестатические члены CEllipseWindow в статической windowProc

	if (windowHandle == NULL) {
		throw std::runtime_error("Fail to CreateWindow " + std::to_string(GetLastError()));
	}
}

void CEllipseWindow::Show(int cmdShow) const
{
	// Show the window and send a WM_PAINT message to the window procedure. 
	ShowWindow(windowHandle, cmdShow);
	UpdateWindow(windowHandle);
}

LRESULT CALLBACK CEllipseWindow::windowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	// ¬осстанавливаем указатель this на CEllipseWindow
	CEllipseWindow* pThis = reinterpret_cast<CEllipseWindow*>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));
    if (pThis == NULL && message != WM_NCCREATE) {
        ShowError("Fail to get pointer to CEllipseWindow " + std::to_string(GetLastError()));
        PostQuitMessage(1);
    }

	switch (message) {
	case WM_NCCREATE:
		// ѕри создании окна, в параметрах сообщени€ WM_NCCREATE будет this
		// ¬ этом случае, его нужно запомнить в параметрах окна.
		pThis = static_cast<CEllipseWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

        SetTimer(windowHandle, 1, 30, NULL);
		return TRUE;
	case WM_PAINT:
		pThis->OnDraw();
		break;
	case WM_ERASEBKGND:
		return static_cast<LRESULT> (1);
	case WM_TIMER:
		pThis->OnTimer();
		break;
    case WM_LBUTTONDOWN:
        SetFocus(windowHandle);
        break;
	case WM_DESTROY:
		pThis->OnDestroy();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(windowHandle, message, wParam, lParam);
	}
	
	return 0;
}

void CEllipseWindow::OnDestroy()
{
    SetWindowLongPtr(windowHandle, GWLP_USERDATA, NULL);
    KillTimer(windowHandle, 1);
}

// TODO: refactor
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
	ellipse.Draw(hdcMem, isFocused());

	BitBlt(context,
		rc.left, rc.top,
		rc.right - rc.left, rc.bottom - rc.top,
		hdcMem, 0, 0, SRCCOPY);

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
        ellipse.Move(rect);
        InvalidateRect(windowHandle, NULL, TRUE);
    } else {
        // ≈сли только что окно перестало быть активным, то нужно перерисовать в неактивном состо€нии
        wasActive = false;
        InvalidateRect(windowHandle, NULL, TRUE);
    }
}
