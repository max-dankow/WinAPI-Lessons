#pragma once
#include <Windows.h>
#include <string>

class CObjectSwitcher {
public:
	CObjectSwitcher(HDC context, HGDIOBJ newObject) : context(context) {
		previous = SelectObject(context, newObject);
	}

	~CObjectSwitcher() {
		SelectObject(context, previous);
	}
private:
	HDC context;
	HGDIOBJ previous;
};

class CEllipse {
public:
	CEllipse(int left, int top, int height, int width) : 
		left(left), top(top), 
		height(height), width(width),
		speedX(10), speedY(10),
		brush(CreateSolidBrush(RGB(128, 0, 128))) { }

	~CEllipse() {
		DeleteObject(brush);
	}

	void draw(HDC context) const {
		CObjectSwitcher penSwitcher(context, GetStockObject(NULL_PEN));
		CObjectSwitcher brushSwitcher(context, brush);
		Ellipse(context, left, top, left + width, top + height);
	}

	void move(RECT border) {
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
	int left, top, height, width;
	int speedX, speedY;
	HBRUSH brush;
};

class COverlappedWindow 
{
public:
	COverlappedWindow(const std::wstring title = L"Main window") : title(title), ellipse(0, 0, 50, 100) { }

	// Зарегистрировать класс окна
	static void RegisterClass();

	// Создать экземпляр окна
	void Create();

	// Показать окно
	void Show(int cmdShow) const;

protected:
	void OnDestroy() { }
	void OnDraw();
	void OnTimer();
private:
	static constexpr wchar_t* ClassName = L"COverlappedWindow";
	static const int TimerElapseMs = 100;

	CEllipse ellipse;
	std::wstring title;
	HWND windowHandle; // хэндл окна
	UINT_PTR timer;

	static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
	void StartTimer() const;
	void StopTimer() const;
};
