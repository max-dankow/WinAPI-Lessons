#include "stdafx.h"
#include "CHeapManager.h"
#include <iostream>

using std::cout;

void Demo1() 
{
	_SYSTEM_INFO systemInfo;
	GetSystemInfo( &systemInfo );
	size_t pageSize = systemInfo.dwPageSize;
	size_t heapSize = 1024 * 1024;

	CHeapManager heapManager;
	heapManager.Create(pageSize, heapSize);
	LPVOID ptr1 = heapManager.Alloc(5000);
	heapManager.Describe();
	LPVOID ptr2 = heapManager.Alloc(50);
	LPVOID ptr3 = heapManager.Alloc(50000);
	LPVOID ptr4 = heapManager.Alloc(100000);
	LPVOID ptr5 = heapManager.Alloc(303000);
	heapManager.Describe();
	heapManager.Destroy();
	heapManager.Describe();
}

int main()
{
	Demo1();
	std::cin.ignore();
    return 0;
}

