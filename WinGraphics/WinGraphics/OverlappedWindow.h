#pragma once
#include <Windows.h>
#include <string>

class COverlappedWindow 
{
public:
	~COverlappedWindow();

	// Зарегистрировать класс окна
	static void RegisterClass();

	// Создать экземпляр окна
	void Create();

	// Показать окно
	void Show(int cmdShow) const;

protected:
	// Не должен кидать исключений
	void OnDestroy() { }

private:
	static const std::string ClassName;
	HWND windowHandle; // хэндл окна
	static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
};
