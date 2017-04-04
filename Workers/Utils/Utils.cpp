#include "Utils.h"
#include <windows.h>

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