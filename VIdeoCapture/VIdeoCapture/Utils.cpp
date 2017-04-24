#include "Utils.h"
#include <sstream>

void ShowError(const std::wstring &message)
{
	MessageBoxW(0, message.c_str(), TEXT("Error"), MB_OK | MB_ICONERROR);
}

std::wstring ErrorMessage(const std::wstring &message, DWORD errorCode) {
    return message + L" " + std::to_wstring(errorCode);
}

std::wstring convertHex(int n)
{
    std::wstringstream stream;
    stream << std::hex << n;
    return stream.str();
}

std::wstring ErrorMessage(HRESULT hr)
{
    TCHAR szErr[MAX_ERROR_TEXT_LEN];
    DWORD res = AMGetErrorText(hr, szErr, MAX_ERROR_TEXT_LEN);
    if (res == 0)
    {
        StringCchPrintf(szErr, MAX_ERROR_TEXT_LEN, TEXT("Unknown Error: 0x%2x"), hr);
    }
    return std::wstring(szErr);
}

void ThrowIfError(const std::wstring tag, HRESULT result) {
    if (FAILED(result)) {
        throw tag + L" (" + convertHex(result) + L") " + ErrorMessage(result);
    }
}
