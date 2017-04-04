#pragma once
#include <windows.h>
#include <string>

std::wstring GetDictionaryPathFromArgs();

std::wstring GenerateName(DWORD processId, const std::wstring &type);

HANDLE GetDataIsReadyEvent(DWORD processId);