#include <windows.h>
#include <string>
#include <iostream>
#include <exception>
#include "../Utils/Utils.h"

static const std::string WORDS_COUNT_LIB = "DllExample.dll";
static const std::string WORDS_COUNT_PROC_NAME = "WordsCount";

typedef int(*WordCountPtr)(const wchar_t *);

HMODULE LoadDll( const std::string &path )
{
    HMODULE dllHandle = LoadLibrary(path.c_str());
    if (dllHandle == 0) {
        throw std::runtime_error("Fail to load dll");
    }
    return dllHandle;
}

int CallFarWordsCount(HMODULE dllHanlde, const std::wstring &text) 
{
    FARPROC wordsCountProc = GetProcAddress(dllHanlde, WORDS_COUNT_PROC_NAME.c_str());
    if( wordsCountProc == 0 ) {
        throw std::runtime_error("Words Count function is not found in the library");
    }
    WordCountPtr WordCount = (WordCountPtr) (wordsCountProc);
    return WordCount(text.c_str());
}

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{

    try {
        std::wstring path = GetFilePathFromArgs();
        std::wstring text = ReadTextFile(path);
        HMODULE dllHandle = LoadDll(WORDS_COUNT_LIB);
        int wordCount = CallFarWordsCount(dllHandle, text);
        ShowWordsCount(wordCount);
        FreeLibrary(dllHandle);
    }
    catch (const std::exception &a) {
        ShowError(a.what());
    }

    return 0;
}