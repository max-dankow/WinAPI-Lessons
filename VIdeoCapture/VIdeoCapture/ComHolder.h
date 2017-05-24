#pragma once

// Обертка над произвольным COM объектом
template <typename T>
class CComHolder {
public:
    CComHolder(T*&& object = nullptr) : object(object) {
        object = nullptr;
    }

    CComHolder(const CComHolder&) = delete;
    CComHolder& operator=(const CComHolder&) = delete;

    CComHolder(CComHolder&& other) {
        this->object = other.object;
        other.object = nullptr;
    }

    CComHolder& operator=(CComHolder&& other) {
        this->object = other.object;
        other.object = nullptr;
        return *this;
    }

    ~CComHolder() {
        if (Exist()) {
            object->Release();
        }
    }

    bool Exist() {
        return object != nullptr;
    }

    void Set(T*&& pointer) {
        if (Exist()) {
            object->Release();
            object = nullptr;
        }
        object = pointer;
        pointer = nullptr;
    }

    T* Object() {
        return object;
    }

private:
    T *object;
};