#include "Utils.h"
#include <Windows.h>

void ShowError(const std::string &message)
{
    MessageBoxA(0, message.c_str(), "Error", MB_OK);
}

void ShowError(const std::wstring &message)
{
    MessageBoxW(0, message.c_str(), L"Error", MB_OK);
}