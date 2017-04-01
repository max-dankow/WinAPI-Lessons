#include <windows.h>
#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <exception>
#include <codecvt>

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

void PrintDictionary(const std::set<std::wstring> &dictionary) {
    std::wcout << "Dictionary:" << std::endl;
    for (std::wstring word : dictionary) {
        std::wcout << word << std::endl;
    }
}


int main(int argc, char* argv[]) {
    std::set<std::wstring> dictionary = ReadDictionary(GetDictionaryPathFromArgs());
    PrintDictionary(dictionary);
    std::cin.ignore();
    return 0;
}