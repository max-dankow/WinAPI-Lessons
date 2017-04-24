#pragma once
#include <windows.h>
#include <dshow.h>
#include <string>

void ShowError(const std::wstring &message);

std::wstring ErrorMessage(const std::wstring &message, DWORD errorCode);
std::wstring ErrorMessage(HRESULT hr);

void ThrowIfError(const std::wstring tag, HRESULT result);