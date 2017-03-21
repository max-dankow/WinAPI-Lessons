#include <windows.h>
#include <string>
#include <iostream>
#include "../DllExample/WordsCount.h"

void ShowWordsCount( int wordsCount ) {
    MessageBox( 0, std::to_string(wordsCount).c_str(), "Words Count", MB_OK );
}

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
    int wordsCount = WordsCount(L"   8f8f8f){");
    ShowWordsCount(wordsCount);
    std::cin.ignore();
    return 0;
}