#include <windows.h>
#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <exception>
#include <codecvt>
#include <assert.h>

#include "../Utils/Utils.h"

std::set<std::wstring> ReadDictionary(const std::wstring &fileName) {
    std::wifstream inputFile(fileName, std::wifstream::in);

    // Магия Юникода, чтобы unicode файл читался как unicode файл
    inputFile.imbue(std::locale(inputFile.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));
    if (inputFile.good()) {
        std::set<std::wstring> dictionary;
        while (!inputFile.eof()) {
            std::wstring word;
            inputFile >> word;
            dictionary.insert(word);
        }
        return dictionary;
    } else {
        throw std::invalid_argument("Failed to read dictionary");
    }
}

void PrintDictionary(const std::set<std::wstring> &dictionary) {
    std::wcout << "Dictionary:" << std::endl;
    for (std::wstring word : dictionary) {
        std::wcout << word << std::endl;
    }
}

std::wstring GetSourceWString()
{
    HANDLE mappingHandle = GetHandleFromArguments(3, "Source mapping handle");
    PVOID mappedFilePtr = MapViewOfFile(mappingHandle, FILE_MAP_READ, 0, 0, 0);
    if (mappedFilePtr == 0) {
        throw std::runtime_error("Fail to map file");
    }
    int offset = GetIntFromArguments(4, "Offset");
    int size = GetIntFromArguments(5, "Size");

    return std::wstring(reinterpret_cast<wchar_t*>(mappedFilePtr));
}

void DoJob() {
    std::wcerr << L"Working HARD! Unicode suck" << std::endl;
}

int main(int argc, char* argv[]) {
    std::wcerr << L"Worker here!" << std::endl;
    std::set<std::wstring> dictionary = ReadDictionary(GetWStringFromArguments(1, "Dictionary file path"));
    HANDLE terminationEvent =  GetHandleFromArguments(2, "Termination Event");
    HANDLE dataIsReadyEvent = GetDataIsReadyEvent(GetCurrentProcessId());
    HANDLE workIsDoneEvent = GetWorkIsDoneEvent(GetCurrentProcessId());

    while (true) {
        HANDLE awaitedEvents[2] = { dataIsReadyEvent, terminationEvent };
        std::wcerr << L"Wait for job" << std::endl;
        DWORD waitResult = WaitForMultipleObjects(2, awaitedEvents, FALSE, INFINITE);
        switch (waitResult) {
        case WAIT_FAILED:
        case WAIT_TIMEOUT:
            assert(false);
        case WAIT_OBJECT_0 + 0:
            // Появилсь работа
            DoJob();
            SetEvent(workIsDoneEvent);
            break;
        case WAIT_OBJECT_0 + 1:
            // Завершение работы
            std::wcerr << L"terminate\n";
            return 0;
        }
    }
}