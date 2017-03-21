#include <exception>

#define WORDS_COUNT_LIB extern "C" __declspec(dllexport)

#include "WordsCount.h"
#include <string>
#include <cctype>
#include <iostream>

int WordsCount( const wchar_t * textPtr )
{
    if( textPtr == 0 ) {
        return 0;
    }
    std::wstring text(textPtr);
    std::wcout << text << std::endl;
    bool isWordStarted = false;
    int wordCount = 0;
    for (wchar_t ch : text) {
        if (islower(tolower(ch))) {
            isWordStarted = true;
        } else {
            if (isWordStarted) {
                wordCount++;
            }
            isWordStarted = false;
        }
    }
    if (isWordStarted) {
        wordCount++;
    }
    
	return wordCount;
}
