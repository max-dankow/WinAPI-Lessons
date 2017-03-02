#include "stdafx.h"
#include <Windows.h>
#include <cstdlib>
#include <iostream>

void printErrorMessage(const DWORD errorCode) {
	HLOCAL string_message = NULL;

	DWORD len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errorCode, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR)&string_message, 0, NULL);

	if (string_message != NULL) {
		std::wcout << (wchar_t *)string_message;
		LocalFree(string_message);
	}
	else {
		std::wcout << "Error message is not found" << std::endl;
	}
}

BOOL tryCommit(LPVOID reserved, SIZE_T commitSize = 0) {
	std::cout << "Trying to commit 0x" << std::hex << commitSize << " bytes:" << std::endl;
	LPVOID committed = VirtualAlloc(reserved, commitSize, MEM_COMMIT, PAGE_READWRITE);
	if (committed == NULL) {
		std::cout << "COMMIT FAIL:" << std::endl;
		printErrorMessage(GetLastError());
		return FALSE;
	} else {
		std::cout << "OK" << std::endl;
		VirtualFree(committed, commitSize, MEM_DECOMMIT);
		return TRUE;
	}
}

BOOL tryAllocate(SIZE_T reserveSize, SIZE_T commitSize=0) {
	std::cout << "Trying to allocate 0x" << std::hex << reserveSize << " bytes:" << std::endl;
	LPVOID buffer = VirtualAlloc(NULL, reserveSize, MEM_RESERVE, PAGE_READWRITE);
	if (buffer == NULL) {
		std::cout << "FAIL:" << std::endl;
		printErrorMessage(GetLastError());
		return FALSE;
	} else {
		std::cout << "OK" << std::endl;
		BOOL commitResult = TRUE;
		if (commitSize != 0) {
			commitResult = tryCommit(buffer, commitSize);
		}
		VirtualFree(buffer, 0, MEM_RELEASE);
		return commitResult;
	}
}

// Определение максимального объема памяти доступного при аллокации.
// Закоммитить можно не больше чем максимально можно зарезервировать.
void exploreMemoryAllocationLimit() {
	// Сначала ищем максимальную еще допустимую степень 2
	SIZE_T bufferSize = 4096;
	SIZE_T base = 4096;
	BOOL result;
	do {
		result = tryAllocate(bufferSize << 1);
		if (result == TRUE) {
			bufferSize <<= 1;
		}
	} while (result == TRUE);

	// Пытаемся жадно добавлять все меньшие степени 2 к размеру резервируемого буффера
	base = bufferSize >> 1;
	do {
		result = tryAllocate(bufferSize + base);
		if (result == TRUE) {
			bufferSize += base;
		}
		base >>= 1;
	} while (base > 0);

	
	// Пытаемся закоммитить максимально возможный для резервирования размер буффера
	// Пытаемся жадно добавлять все меньшие степени 2 к размеру резервируемого буффера
	SIZE_T commitSize = 0;
	base = bufferSize;
	do {
		result = tryAllocate(bufferSize, commitSize + base);
		if (result == TRUE) {
			commitSize += base;
		}
		base >>= 1;
	} while (base > 0);

	// Демонстрация результата для MEM_RESERVE
	tryAllocate(bufferSize);
	tryAllocate(bufferSize + 1);
	// Демонстрация результата для MEM_COMMIT
	tryAllocate(bufferSize, commitSize);
	tryAllocate(bufferSize, commitSize + 1);
	std::cout << "Max reserve size = " << std::dec << (bufferSize >> 20) << "MB" << std::endl;
	std::cout << "Max commit size = " << std::dec << (commitSize >> 20) << "MB" << std::endl;
}

void demonstrateMemoryFragmentation(SIZE_T allocationNumber) {
	for (SIZE_T i = 0; i < allocationNumber; ++i) {
		SIZE_T reserveSize = (rand() % 128 + 10) * 1024;
		SIZE_T commitSize = ((rand() * 1024) % reserveSize);
		LPVOID reserved = VirtualAlloc(NULL, reserveSize, MEM_RESERVE, (rand() % 3 == 0)? PAGE_GUARD : PAGE_READWRITE);
		VirtualAlloc(reserved, commitSize, MEM_COMMIT, PAGE_READWRITE);
	}
	std::cin.ignore();
}

// Сколько HANDLE может быть создано у процесса.
// "The per-process limit on kernel hanles is 2^24, but actual limit is based on available memory limit"
void exploreHandlersLimit() {
	HANDLE eventHandle = CreateEvent(NULL, TRUE, TRUE, NULL);
	std::cout << "Original event HANDLE " << eventHandle << std::endl;
	std::cout << "Computing hanles limit..." << std::endl;
	SIZE_T count = 1;
	BOOL result;
	do {
		HANDLE duplicatedHandle; 
		result = DuplicateHandle(GetCurrentProcess(), eventHandle, GetCurrentProcess(), &duplicatedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
		if (result == TRUE) {
			count++;
		} else {
			printErrorMessage(GetLastError());
		}
	} while (result == TRUE);
	std::cout << "Handles limit is " << count << std::endl;
}

// Сколько объектов Event удасться создать в одном процессе.
// Логично, что не более максимального числа HANDLE.
void exploreKernelObjectsLimit() {
	std::cout << "Computing kernel objects limit..." << std::endl;
	SIZE_T count = 1;
	HANDLE eventHandle;
	do {
		eventHandle = CreateEvent(NULL, TRUE, TRUE, NULL);
		if (eventHandle != NULL) {
			count++;
		}
		else {
			printErrorMessage(GetLastError());
		}
	} while (eventHandle != NULL);
	std::cout << "CreateEvent limit is " << count << std::endl;
}

// GDI объекты допускают только один HANLDE на поток. К тому же они являются приватными для потока их создавшего.
// "There is a theoretical limit of 65, 536 GDI handles per session. 
// However, the maximum number of GDI handles that can be opened per session is usually lower, 
// since it is affected by available memory."
// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows\GDIProcessHandleQuota = 10000
void exploreGdiObjectsLimit() {
	std::cout << "Computing GDI objects limit..." << std::endl;
	SIZE_T count = 1;
	HANDLE penHandle;
	do {
		penHandle = CreatePen(0, 0, NULL);
		if (penHandle != NULL) {
			count++;
		}
		else {
			printErrorMessage(GetLastError());
		}
	} while (penHandle != NULL);
	std::cout << "CreateEvent limit is " << count << std::endl;
}

void recursive(SIZE_T prevAddr) {
	BYTE local[400]; // 100 - 328; 1 - 228
	std::cout << prevAddr - (int)&local << std::endl;
	recursive((int)&local);
}

void exploreCallSize() {
	// 512b virtual mem for stack - 1113 calls
	// 1 048 576 / 4354 = 240
	// 1 048 5760 / 48500 = 216
	// 2 097 152 / 9608 = 218
	// CallSize is around 217 bytes(267 if with local array)
	recursive(0);
}

int main() {
	exploreMemoryAllocationLimit();
	//exploreHandlersLimit();
	//exploreKernelObjectsLimit();
	//exploreGdiObjectsLimit(); // 9997 (actual quota is 10000)
	//demonstrateMemoryFragmentation(1000);
	//exploreCallSize();
	return 0;
}

