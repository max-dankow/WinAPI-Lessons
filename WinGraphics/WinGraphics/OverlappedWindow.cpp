#include "OverlappedWindow.h"
#include <exception>
#include <string>

class CContext
{
public:
	CContext(HWND hWnd, LPPAINTSTRUCT paintStruct) : window(hWnd), paintStruct(paintStruct) {
		context = BeginPaint(hWnd, paintStruct);
	}
	~CContext() {
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

const std::string COverlappedWindow::ClassName = "COverlappedWindow";

COverlappedWindow::~COverlappedWindow()
{
	OnDestroy();
}

// Стоит заметить, что благодаря define RegisterClass RegisterClassW, теперь везде происходит замена.
void COverlappedWindow::RegisterClass()
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
	wcx.lpszClassName = L"MainWClass";
	wcx.hIconSm = NULL;
	if (RegisterClassExW(&wcx) == 0) {
		throw std::runtime_error("Fail to RegisterClass " + std::to_string(GetLastError()));
	}
}

void COverlappedWindow::Create()
{
	windowHandle = CreateWindow(
	L"MainWClass",  // name of window class
	L"Sample",  // title-bar string
	WS_OVERLAPPEDWINDOW,  // top-level window
	CW_USEDEFAULT,  // default horizontal position
	CW_USEDEFAULT,  // default vertical position
	CW_USEDEFAULT,  // default width
	CW_USEDEFAULT,  // default height
	(HWND)NULL,  // no owner window
	(HMENU)NULL,  // use class menu
	GetModuleHandle(NULL),  // handle to current application instance
	(LPVOID)NULL);  // no window-creation data

	if (windowHandle == NULL) {
		throw std::runtime_error("Fail to CreateWindow " + std::to_string(GetLastError()));
	}
}

void COverlappedWindow::Show(int cmdShow) const
{
	// Show the window and send a WM_PAINT message to the window procedure. 
	ShowWindow(windowHandle, cmdShow);
	UpdateWindow(windowHandle);
}


LRESULT CALLBACK COverlappedWindow::windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_PAINT: {
		PAINTSTRUCT paintStruct;
		CContext contextHolder(handle, &paintStruct);
		HDC context = contextHolder.getContext();
		TextOut(context, 0, 0, L"TextOut", sizeof(L"TextOut"));
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(handle, message, wParam, lParam);
	}
	return 0;
}