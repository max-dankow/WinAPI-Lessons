#pragma once

#ifdef WORDS_COUNT_LIB

#else
// � ���� ������ ������������ ���� ���������� � ������������ ������
#define WORDS_COUNT_LIB extern "C" __declspec(dllimport)
	
#endif  // WORDS_COUNT_LIB


// ������� ���������� ���� � ������ text
WORDS_COUNT_LIB int WordsCount( const wchar_t* text );