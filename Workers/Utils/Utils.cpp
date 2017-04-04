#include "Utils.h"
#include <windows.h>

static const std::wstring NamePrefix(L"ru.mipt.diht.dankovtsev.workers.");

std::wstring GetDictionaryPathFromArgs()
{
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc < 2) {
        throw std::invalid_argument("There should be a path to unicode dictionary file as argument");
    }
    else {
        return std::wstring(argv[1]);
    }
}

std::wstring GenerateName(DWORD processId, const std::wstring &type)
{
    return std::wstring(NamePrefix).append(type).append(std::to_wstring(processId));
}

HANDLE GetDataIsReadyEvent(DWORD processId) 
{
    auto name = GenerateName(processId, L"DataIsReady");
    auto dataIsReadyEvent = CreateEvent(0, FALSE, FALSE, name.c_str());
    if (dataIsReadyEvent == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to create event");
    }
    return dataIsReadyEvent;
}

HANDLE GetWorkIsDoneEvent(DWORD processId)
{
    auto name = GenerateName(processId, L"WorkIsDone");
    auto workIsDoneEvent = CreateEvent(0, FALSE, FALSE, name.c_str());
    if (workIsDoneEvent == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to create event");
    }
    return workIsDoneEvent;
}