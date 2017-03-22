#include "stdafx.h"
#include "CHeapManager.h"
#include "CRandomSizeClass.h"
#include <iostream>
#include <time.h>
#include <chrono>

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

template<typename T>
void TestHeap(int numberOfAllocations)
{
    using std::chrono::high_resolution_clock;

    T::CreateHeap(numberOfAllocations * MAX_SIZE);
    std::vector<T*> pointers;
    pointers.reserve(numberOfAllocations);
    high_resolution_clock::time_point momentBeforeAllocations = high_resolution_clock::now();
    for (int i = 0; i < numberOfAllocations; ++i) {
        pointers.push_back(new T);
    }
    high_resolution_clock::time_point momentBeforeFree = high_resolution_clock::now();
    //heapManager.Describe();
    for (T* pointer : pointers) {
        delete pointer;
    }
    T::DestroyHeap();
    high_resolution_clock::time_point momentFinal = high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(momentFinal - momentBeforeAllocations).count();
    auto allocDuration = std::chrono::duration_cast<std::chrono::milliseconds>(momentBeforeFree - momentBeforeAllocations).count();
    auto freeDuration = std::chrono::duration_cast<std::chrono::milliseconds>(momentFinal - momentBeforeFree).count();
    std::cout << "Total:\t" << totalDuration << std::endl;
    std::cout << "Alloc:\t" << allocDuration << std::endl;
    std::cout << "Free:\t"<< freeDuration << std::endl;
}

void CompareTime()
{
    int numerOfAllocs = 2000;
    std::cout << "Custom" << std::endl;
    TestHeap<CCustomRandomSizeClass>( numerOfAllocs );

    std::cout << std::endl << "Standart" << std::endl;
    TestHeap<CRandomSizeClass>( numerOfAllocs );
}

int main()
{
    CompareTime();

    //CRandomSizeClass::CreateHeap(64 * 1024 * 1024);
	std::cin.ignore();
    return 0;
}

