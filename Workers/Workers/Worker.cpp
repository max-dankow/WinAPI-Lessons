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

std::wstring GetSourceWString(CMappedFile &mappedFile)
{
    HANDLE mappingHandle = GetSourceMapping(GetCurrentProcessId());
    PVOID mappedFilePtr = MapViewOfFile(mappingHandle, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (mappedFilePtr == 0) {
        throw std::runtime_error("Fail to map file");
    }

    mappedFile = std::move(CMappedFile(INVALID_HANDLE_VALUE, mappingHandle, mappedFilePtr, 0));

    return std::wstring(reinterpret_cast<wchar_t*>(mappedFilePtr));
}

void TheStupidestFilter(const std::wstring &source, const std::set<std::wstring> &dictioinary, wchar_t *dest) {
    size_t i = 0;

    while (i < source.length()) {
        bool isGoodWord = true;
        for (const std::wstring &word : dictioinary) {
            if (i + word.length() <= source.length() && source.substr(i, word.length()) == word) {
                i += word.length();
                isGoodWord = false;
                break;
            }
        }
        if (isGoodWord) {
            *dest = source.at(i);
            dest++;
            ++i;
        }
    }
    *dest = L'\0';
}

void DoJob(const std::set<std::wstring> &dictioinary) {
    CMappedFile mappedFile;
    auto source = GetSourceWString(mappedFile);
    TheStupidestFilter(source, dictioinary, reinterpret_cast<wchar_t*>(mappedFile.GetAddr()));
}

int main(int argc, char* argv[]) {
    std::set<std::wstring> dictionary = ReadDictionary(GetWStringFromArguments(1, "Dictionary file path"));
    HANDLE terminationEvent =  GetHandleFromArguments(2, "Termination Event");
    HANDLE dataIsReadyEvent = GetDataIsReadyEvent(GetCurrentProcessId());
    HANDLE workIsDoneEvent = GetWorkIsDoneEvent(GetCurrentProcessId());

    while (true) {
        HANDLE awaitedEvents[2] = { dataIsReadyEvent, terminationEvent };
        DWORD waitResult = WaitForMultipleObjects(2, awaitedEvents, FALSE, INFINITE);
        switch (waitResult) {
        case WAIT_FAILED:
        case WAIT_TIMEOUT:
            assert(false);
        case WAIT_OBJECT_0 + 0:
            // Появилсь работа
            DoJob(dictionary);
            SetEvent(workIsDoneEvent);
            break;
        case WAIT_OBJECT_0 + 1:
            // Завершение работы
            return 0;
        }
    }
}