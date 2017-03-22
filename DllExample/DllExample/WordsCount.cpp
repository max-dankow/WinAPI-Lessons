#include <exception>

// Из этого модуля экспортируем
#define WORDS_COUNT_LIB extern "C" __declspec(dllexport)

#include "WordsCount.h"
#include <string>
#include <cctype>
#include <iostream>

int WordsCount( const wchar_t * text )
{
    if( text == 0 ) {
        return 0;
    }
    bool isWordStarted = false;
    int wordCount = 0;
    for (size_t i = 0; text[i] != L'\0'; ++i) {
        if( iswlower( towlower( text[i] ) ) ) {
            isWordStarted = true;
        } else {
            if( isWordStarted ) {
                wordCount++;
            }
            isWordStarted = false;
        }
    }
    if ( isWordStarted ) {
        wordCount++;
    }
    
	return wordCount;
}
