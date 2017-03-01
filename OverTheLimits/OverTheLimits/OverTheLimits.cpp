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
	}
	else {
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

// Определение максимального объема памяти доступного при аллокации
void exploreMemoryAllocationLimit() {
	SIZE_T bufferSize = 4096;
	SIZE_T base = 4096;
	BOOL result;
	do {
		bufferSize <<= 1;
		result = tryAllocate(bufferSize);
	} while (result == TRUE);
	bufferSize >>= 1;
	base = bufferSize >> 1;
	do {
		do {
			result = tryAllocate(bufferSize + base);
			if (result == TRUE) {
				bufferSize += base;
			}
		} while (result == TRUE);
		base >>= 1;
	} while (base > 0);
	tryAllocate(bufferSize);
	tryAllocate(bufferSize + 1);
	std::cout << "Max reserve size = " << std::dec << bufferSize / 1024 / 1024 << "MB" << std::endl;
	tryAllocate(bufferSize, bufferSize);
	tryAllocate(bufferSize, bufferSize + 1);
	std::cout << "Max commit size = " << std::dec << bufferSize / 1024 / 1024 << "MB" << std::endl;
}

int main() {
	exploreMemoryAllocationLimit();
    return 0;
}

