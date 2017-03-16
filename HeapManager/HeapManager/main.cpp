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
	heapManager.Alloc(5000);
	heapManager.Alloc(50);
}

int main()
{
	Demo1();
	std::cin.ignore();
    return 0;
}

