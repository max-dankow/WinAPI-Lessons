#pragma once
#include <Windows.h>
#include <string>

class CEllipse {
public:
	CEllipse(int left, int top, int height, int width) : 
		left(left), top(top), 
		height(height), width(width),
		brush(CreateSolidBrush(RGB(128, 0, 128))) { }

	~CEllipse() {
		DeleteObject(brush);
	}

	void draw(HDC context) const {
		SelectObject(context, GetStockObject(NULL_PEN));
		SelectObject(context, brush);
		Ellipse(context, left, top, left + width, top + height);
	}

	void move() {
		top += 1;
		left += 1;
	}
private:
	int left, top, height, width;
	HBRUSH brush;
};

class COverlappedWindow 
{
public:
	COverlappedWindow(const std::wstring title = L"Main window") : title(title), ellipse(0, 0, 50, 100) { }

	// ���������������� ����� ����
	static void RegisterClass();

	// ������� ��������� ����
	void Create();

	// �������� ����
	void Show(int cmdShow) const;

protected:
	void OnDestroy() { }
	void OnDraw();
	void OnTimer();
private:
	static constexpr wchar_t* ClassName = L"COverlappedWindow";
	static const int TimerElapseMs = 16;

	CEllipse ellipse;
	std::wstring title;
	HWND windowHandle; // ����� ����
	UINT_PTR timer;
	static HDC memDC;

	static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
	void StartTimer() const;
	void StopTimer() const;
};
