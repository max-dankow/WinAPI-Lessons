#pragma once
#include <Windows.h>
#include <string>

class COverlappedWindow 
{
public:
	~COverlappedWindow();

	// ���������������� ����� ����
	static void RegisterClass();

	// ������� ��������� ����
	void Create();

	// �������� ����
	void Show(int cmdShow) const;

protected:
	// �� ������ ������ ����������
	void OnDestroy() { }

private:
	static const std::string ClassName;
	HWND windowHandle; // ����� ����
	static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
};
