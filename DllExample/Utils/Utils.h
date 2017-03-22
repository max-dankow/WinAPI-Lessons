#pragma once

#include <string>


void ShowWordsCount( int wordsCount );
void ShowError( const std::string &message );
std::wstring ReadTextFile( const std::wstring &path );
std::wstring GetFilePathFromArgs();