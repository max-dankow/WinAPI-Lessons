#pragma once
#include <windows.h>
#include <iostream>
#include <vector>

class CHeapManager {

public:
	CHeapManager();
	~CHeapManager();

	// ����������� ����������� ������ ������ �������� maxSize, 
	// ������������� ����� minSize ���������� ������
	void Create( size_t minSize, size_t maxSize );
	void* Alloc( int size );
	void Free( void* mem );

private:
	static size_t round(size_t value, size_t roundTo);
	void initializePages();

	_SYSTEM_INFO systemInfo;
	LPVOID heap;
	size_t heapSize;
	// ���������� ������ �� ������ �� �������
	std::vector<int> pages;

};
