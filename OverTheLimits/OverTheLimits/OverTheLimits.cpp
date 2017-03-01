#include "stdafx.h"
#include <Windows.h>
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

BOOL tryCommit(LPVOID reservedBuffer, SIZE_T commitSize = 0) {
	std::cout << "Trying to commit 0x" << std::hex << commitSize << " bytes:" << std::endl;
	LPVOID buffer = VirtualAlloc(NULL, commitSize, MEM_COMMIT, PAGE_READWRITE);
	if (buffer == NULL) {
		std::cout << "COMMIT FAIL:" << std::endl;
		printErrorMessage(GetLastError());
		VirtualFree(buffer, commitSize, MEM_DECOMMIT);
		return FALSE;
	} else {
		std::cout << "OK" << std::endl;
		VirtualFree(buffer, commitSize, MEM_DECOMMIT);
		return TRUE;
	}
}

BOOL tryAllocate(SIZE_T reserveSize, SIZE_T commitSize=0) {
	std::cout << "Trying to allocate 0x" << std::hex << reserveSize << " bytes:" << std::endl;
	LPVOID buffer = VirtualAlloc(NULL, reserveSize, MEM_RESERVE, PAGE_READWRITE);
	if (buffer == NULL) {
		std::cout << "FAIL:" << std::endl;
		printErrorMessage(GetLastError());
		VirtualFree(buffer, 0, MEM_RELEASE);
		return FALSE;
	} else {
		std::cout << "OK" << std::endl;
		BOOL commitResult = TRUE;
		if (commitSize != 0) {
			//VirtualAlloc(buffer, commitSize, MEM_COMMIT, PAGE_READWRITE);
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

	// Демонстрация результата для MEM_RESERVE
	tryAllocate(bufferSize);
	tryAllocate(bufferSize + 1);
	std::cout << "Max reserve size = " << std::dec << bufferSize / 1024 / 1024 << "MB" << std::endl;

	// Пытаемся закоммитить максимально возможный для резервирования размер буффера
	// Демонстрация результата для MEM_COMMIT
	tryAllocate(bufferSize, bufferSize);
	tryAllocate(bufferSize, bufferSize + 1);
	std::cout << "Max commit size = " << std::dec << bufferSize / 1024 / 1024 << "MB" << std::endl;
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

int main() {
	//exploreMemoryAllocationLimit();
	//exploreHandlersLimit();
	//exploreKernelObjectsLimit();
	exploreGdiObjectsLimit(); // 9997
	return 0;
}

