#pragma once
#include <windows.h>

class CHandle {
public:
    CHandle() { }
    CHandle(HANDLE&& handle) {
        this->handle = handle;
        handle = INVALID_HANDLE_VALUE;
    }

    CHandle(CHandle&& other) {
        this->handle = other.handle;
        other.invalidate();
    }

    CHandle& operator= (CHandle&& other) {
        Close();
        this->handle = other.handle;
        other.invalidate();
        return *this;
    }

    CHandle(const CHandle&) = delete;
    CHandle& operator= (CHandle&) = delete;

    ~CHandle() {
        Close();
    }

    bool IsValid() {
        return handle == INVALID_HANDLE_VALUE;
    }

    HANDLE Get() const {
        return handle;
    }

    void Close() {
        if (IsValid()) {
            CloseHandle(handle);
        }
    }
private:
    HANDLE handle;

    void invalidate() {
        handle = INVALID_HANDLE_VALUE;
    }
};