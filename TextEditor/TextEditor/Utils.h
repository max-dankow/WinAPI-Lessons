#pragma once
#include <windows.h>
#include <string>

template <typename T>
class CGdiObject {
public:
    CGdiObject() { }
    CGdiObject(T&& newObject) {
        this->object = newObject;
        newObject = NULL;
    }

    CGdiObject(CGdiObject&& other) {
        this->object = other.object;
        other.invalidate();
    }

    CGdiObject& operator= (T&& newObject) {
        Close();
        this->object = newObject;
        newObject = NULL;
        return *this;
    }

    CGdiObject& operator= (CGdiObject&& other) {
        Close();
        this->object = other.object;
        other.invalidate();
        return *this;
    }

    CGdiObject(const CGdiObject&) = delete;
    CGdiObject& operator= (CGdiObject&) = delete;

    ~CGdiObject() {
        Close();
    }

    bool IsValid() {
        return object != NULL;
    }

    T Get() const {
        return object;
    }

    void Close() {
        if (IsValid()) {
            DeleteObject(object);
        }
    }
private:
    T object;

    void invalidate() {
        object = NULL;
    }
};
// TODO: dry
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

void ShowError(const std::string &message);
void ShowError(const std::wstring &message);
