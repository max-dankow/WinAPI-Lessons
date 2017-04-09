#include "Utils.h"
#include <windows.h>

static const std::wstring NamePrefix(L"ru.mipt.diht.dankovtsev.workers.");

std::wstring GetArgument(size_t index, const std::string &what)
{
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc < index + 1) {
        throw std::invalid_argument(std::string(what).append(" expetced as argument"));
    } else {
        return std::wstring(argv[index]);
    }
}

std::wstring GetWStringFromArguments(size_t index, const std::string &what)
{
    return GetArgument(index, what);
}

int GetIntFromArguments(size_t index, const std::string &what)
{
    return std::stoi(GetArgument(index, what));
}

HANDLE GetHandleFromArguments(size_t index, const std::string &what)
{
    return reinterpret_cast<HANDLE> (GetIntFromArguments(index, what));
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

HANDLE GetSourceMapping(DWORD processId, size_t size) 
{
    auto name = GenerateName(processId, L"SourceMapping");
    HANDLE sourceMapping;
    if (size != 0) {
        sourceMapping = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, size, name.c_str());
    } else {
        sourceMapping = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, name.c_str());
    }
    if (sourceMapping == 0) {
        auto lerr = GetLastError();
        throw std::runtime_error("Failed to create mapping");
    }
    return sourceMapping;
}