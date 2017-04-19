#pragma once
#include <Windows.h>

class COverlappedWindow 
{
public:
	COverlappedWindow();
	~COverlappedWindow();
	// Зарегистрировать класс окна
	static bool RegisterClass();
	// Создать экземпляр окна
	bool Create();
	// Показать окно
	void Show(int cmdShow);
protected:
	void OnDestroy();
private:
	HWND handle; // хэндл окна
	static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
};
