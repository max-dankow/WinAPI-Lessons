#pragma once
#include <windows.h>
#include <string>

std::wstring GetArgument(size_t index, const std::string &what);

std::wstring GetWStringFromArguments(size_t index, const std::string &what);

HANDLE GetHandleFromArguments(size_t index, const std::string &what);

std::wstring GenerateName(DWORD processId, const std::wstring &type);

HANDLE GetDataIsReadyEvent(DWORD processId);

HANDLE GetWorkIsDoneEvent(DWORD processId);

int GetIntFromArguments(size_t index, const std::string &what);