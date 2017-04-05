#pragma once
#include <windows.h>
#include <string>


struct CMappedFile {
    HANDLE fileHandle, mappingHandle;
    PVOID mappedFilePtr;
    size_t size;
};

void OnTerminate(CMappedFile &mappedFile);

std::wstring GetArgument(size_t index, const std::string &what);

std::wstring GetWStringFromArguments(size_t index, const std::string &what);

HANDLE GetHandleFromArguments(size_t index, const std::string &what);

std::wstring GenerateName(DWORD processId, const std::wstring &type);

HANDLE GetDataIsReadyEvent(DWORD processId);

HANDLE GetWorkIsDoneEvent(DWORD processId);

HANDLE GetSourceMapping(DWORD processId, size_t size = 0);

int GetIntFromArguments(size_t index, const std::string &what);