#include <windows.h>
#include <string>
#include <iostream>
#include <exception>
#include "../StaticLib/WordsCount.h"
#include "../Utils/Utils.h"

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{

    try {
        std::wstring path = GetFilePathFromArgs();
        std::wstring text = ReadTextFile(path);
        ShowWordsCount(WordsCount(text.c_str()));
    } catch ( const std::exception &a ) {
        ShowError(a.what());
    }
    
    return 0;
}