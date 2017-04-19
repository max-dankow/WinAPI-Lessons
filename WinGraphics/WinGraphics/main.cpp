#include <windows.h> 
#include <iostream>
#include <string>
#include <exception>

// Global variable 

HINSTANCE hinst;

// Function prototypes. 

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
void InitApplication(HINSTANCE);
void InitInstance(HINSTANCE, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

void ShowError(const std::string &message)
{
	MessageBoxA(0, message.c_str(), "Error", MB_OK);
}

// Application entry point.

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	try {
		InitApplication(hinstance);
		InitInstance(hinstance, nCmdShow);
	} catch (std::runtime_error e) {
		ShowError(e.what());
	}

	BOOL fGotMessage;
	while ((fGotMessage = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0 && fGotMessage != -1)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
	UNREFERENCED_PARAMETER(lpCmdLine);
}

void InitApplication(HINSTANCE hinstance)
{
	WNDCLASSEX wcx;

	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = MainWndProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hinstance;
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

void InitInstance(HINSTANCE hinstance, int nCmdShow)
{
	HWND hwnd;

	// Save the application-instance handle. 

	hinst = hinstance;

	// Create the main window. 

	hwnd = CreateWindow(
		L"MainWClass",        // name of window class 
		L"Sample",            // title-bar string 
		WS_OVERLAPPEDWINDOW, // top-level window 
		CW_USEDEFAULT,       // default horizontal position 
		CW_USEDEFAULT,       // default vertical position 
		CW_USEDEFAULT,       // default width 
		CW_USEDEFAULT,       // default height 
		(HWND)NULL,         // no owner window 
		(HMENU)NULL,        // use class menu 
		hinstance,           // handle to application instance 
		(LPVOID)NULL);      // no window-creation data 

	if (hwnd == NULL) {
		throw std::runtime_error("Fail to CreateWindow " + std::to_string(GetLastError()));
	}

	// Show the window and send a WM_PAINT message to the window 
	// procedure. 

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
}

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

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	switch (message) {
	case WM_PAINT: {
		PAINTSTRUCT paintStruct;
		CContext contextHolder(hWnd, &paintStruct);
		HDC context = contextHolder.getContext();
		TextOut(context, 0, 0, L"TextOut", sizeof(L"TextOut"));
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default: 
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}