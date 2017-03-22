#pragma once
#include "CHeapManager.h"
#include <windows.h>
#include <random>

static const int MAX_SIZE = 256 * 1024;
static HANDLE standartHeap;
static CHeapManager heapManager;

static std::default_random_engine randomEngine;

static size_t GenerateSize()
{
    static const size_t LOWER[3] = { 0, 4*1024, 128*1024 };
    static const size_t UPPER[3] = { 4*1024, 128*1024, MAX_SIZE };

    std::uniform_int_distribution<size_t> typeDistribution(0, 2);
    int sizeType = typeDistribution(randomEngine);
    std::uniform_int_distribution<size_t> sizeDistribution(LOWER[sizeType], UPPER[sizeType]);
    return sizeDistribution(randomEngine);
}

class CRandomSizeClass {


public:
    static void CreateHeap(size_t heapSize) 
    {
        standartHeap = HeapCreate(0, 4*1024, heapSize);
    }

    void* operator new(size_t size)
    {
        return HeapAlloc(standartHeap, 0, size * GenerateSize());
    }

    void operator delete( void* ptr ) 
    {
        HeapFree(standartHeap, 0, ptr);
    }

    static void DestroyHeap()
    {
        HeapDestroy(standartHeap);
    }

};

class CCustomRandomSizeClass {

public:
    static void CreateHeap(size_t heapSize) 
    {
        heapManager.Create(4*1024, heapSize);
    }

    void* operator new(size_t size)
    {
        return heapManager.Alloc(size * GenerateSize());
    }

    void operator delete(void* ptr) {
        heapManager.Free(ptr);
    }

    static void DestroyHeap()
    {
        heapManager.Destroy();
    }

};

