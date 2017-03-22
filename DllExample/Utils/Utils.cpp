#include "Utils.h"

#include <windows.h>

// Ограничение на размер открываеого файла, можно изменять
static const size_t MAX_TEXT_FILE_SIZE = 256 * 1024 * 1024;

void ShowWordsCount(int wordsCount) 
{
    MessageBox(0, std::to_string(wordsCount).c_str(), "Words Count", MB_OK);
}

void ShowError(const std::string &message) 
{
    MessageBox(0, message.c_str(), "Error", MB_OK);
}

std::wstring ReadTextFile(const std::wstring &path)
{
    HANDLE fileHandle = CreateFileW(path.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        DWORD c = GetLastError();
        throw std::runtime_error("Can't open file");
    }

    DWORD fileSize = GetFileSize(fileHandle, &fileSize);
    if (fileSize > MAX_TEXT_FILE_SIZE) {
        throw std::runtime_error("File is too big");
    }
    size_t length = fileSize / sizeof(wchar_t);
    wchar_t* buffer = new wchar_t[length + 1];
    buffer[length] = L'\0';
    DWORD numberOfBytesRead;

    if (!ReadFile(fileHandle, buffer, fileSize, &numberOfBytesRead, 0) || numberOfBytesRead != fileSize) {
        throw std::runtime_error("Fail to read file");
    }

    CloseHandle(fileHandle);
    std::wstring text(buffer);
    return text;
}

std::wstring GetFilePathFromArgs()
{
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc < 2) {
        throw std::invalid_argument("There should be a path to unicode text file as argument");
    }
    else {
        return std::wstring(argv[1]);
    }
}