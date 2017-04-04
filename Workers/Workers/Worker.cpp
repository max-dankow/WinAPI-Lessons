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

HANDLE GetParentTerminationHandle()
{
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    assert(argc >= 3);
    return reinterpret_cast<HANDLE> (std::stoi(argv[2]));
}

int main(int argc, char* argv[]) {
    std::cerr << "Worker here!" << std::endl;
    std::set<std::wstring> dictionary = ReadDictionary(GetDictionaryPathFromArgs());
    HANDLE terminationEvent = GetParentTerminationHandle();
    HANDLE dataIsReadyEvent = GetDataIsReadyEvent(GetCurrentProcessId());
    HANDLE workIsDoneEvent = GetWorkIsDoneEvent(GetCurrentProcessId());

    while (true) {
        HANDLE awaitedEvents[2] = { dataIsReadyEvent, terminationEvent };
        std::cerr << "Wait for job" << std::endl;
        DWORD waitResult = WaitForMultipleObjects(2, awaitedEvents, FALSE, INFINITE);
        switch (waitResult) {
        case WAIT_FAILED:
        case WAIT_TIMEOUT:
            assert(false);
        case WAIT_OBJECT_0 + 0:
            // Появилсь работа
            std::cerr << "Working HARD!" << std::endl;
            SetEvent(workIsDoneEvent);
            break;
        case WAIT_OBJECT_0 + 1:
            // Завершение работы
            std::cerr << "terminate\n";
            return 0;
        }
    }
}