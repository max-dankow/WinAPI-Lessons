#pragma once
#include <windows.h>
#include <string>


class CMappedFile {
public:
    CMappedFile(HANDLE fileHandle, HANDLE mappingHandle, PVOID mappedFilePtr,  size_t size):
        fileHandle(fileHandle),
        mappingHandle(mappingHandle),
        mappedFilePtr(mappedFilePtr),
        size(size) { }

    CMappedFile() {
        invalidate();
    }

    CMappedFile(const CMappedFile &) = delete;
    void operator=(const CMappedFile&) = delete;

    CMappedFile(CMappedFile&& other) :
        fileHandle(other.fileHandle),
        mappingHandle(other.mappingHandle),
        mappedFilePtr(other.mappedFilePtr),
        size(other.size)
    {
        other.invalidate();
    }

    void operator=(CMappedFile&& other) {
        fileHandle = other.fileHandle;
        mappingHandle = other.mappingHandle;
        mappedFilePtr = other.mappedFilePtr;
        size = other.size;
        other.invalidate();
    }

    ~CMappedFile() {
        if (fileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(fileHandle);
        }
        if (mappingHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(mappingHandle);
        }
        UnmapViewOfFile(mappedFilePtr);
    }

    size_t GetSize() const {
        return size;
    }

    PVOID GetAddr() const {
        return mappedFilePtr;
    }

private:
    HANDLE fileHandle, mappingHandle;
    PVOID mappedFilePtr;
    size_t size;

    void invalidate() {
        fileHandle = INVALID_HANDLE_VALUE;
        mappingHandle = INVALID_HANDLE_VALUE;
        mappedFilePtr = NULL;
    }
};

std::wstring GetArgument(size_t index, const std::string &what);

std::wstring GetWStringFromArguments(size_t index, const std::string &what);

HANDLE GetHandleFromArguments(size_t index, const std::string &what);

std::wstring GenerateName(DWORD processId, const std::wstring &type);

HANDLE GetDataIsReadyEvent(DWORD processId);

HANDLE GetWorkIsDoneEvent(DWORD processId);

HANDLE GetSourceMapping(DWORD processId, size_t size = 0);

int GetIntFromArguments(size_t index, const std::string &what);