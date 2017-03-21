#pragma once

#ifdef WORDS_COUNT_LIB

#else
// В этом случае заголовочный файл включается в использующий модуль
#define WORDS_COUNT_LIB extern "C" __declspec(dllimport)
	
#endif  // WORDS_COUNT_LIB


// Считает количество слов в тексте text
WORDS_COUNT_LIB int WordsCount( const wchar_t* text );