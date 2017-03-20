#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
#include <array>
#include <set>
#include <map>

struct Heading {
	Heading(Heading* prev, size_t size) {
		this->prev = prev;
		this->blockSize = size;
	}
	Heading* prev;
	size_t blockSize;
};

struct Block {

	Block(LPVOID addr, size_t size) {
		this->addr = addr;
		this->size = size;
	}

	bool operator<(Block const & b){
		return this->addr < b.addr;
	}

	LPVOID addr;
	// ������ ��� ����� ����������
	size_t size;
};

struct BlockComparator {
	bool operator()(Block const & a, Block const & b) const
	{
		return (a.size < b.size) || (a.size == b.size && a.addr < b.addr);
	}
};

struct AddrComparator {
	bool operator()(Block const & a, Block const & b) const
	{
		return a.addr < b.addr;
	}
};


class CHeapManager {

public:
	CHeapManager();

	// ����������� ����������� ������ ������ �������� maxSize, 
	// ������������� ����� minSize ���������� ������
	void Create( size_t minSize, size_t maxSize );
	void* Alloc( size_t size );
	void Free( void* mem );
	void Describe();

private:
	static size_t round( size_t value, size_t roundTo );
	static size_t getSizeType(size_t size);
	void initializePageUsageCounters();
	Block findSuitableFreeBlock(size_t size);
	void mergeNext(Block block);
	void ensureBlockIsCommitted(const Block block);
	void releasePage(LPVOID);
	void updatePages(const Block block, int sign);
	Block biteOfNewBlock(const Block source, size_t size);
	void addFreeBlock(const Block);
	void removeFreeBlock(const Block);
	void allocateBlock(const Block);
	Heading* getHeadingAddr(const Block) const;
	LPVOID getBlockBodyAddr(const Block) const;
	LPVOID getNext(const Block) const;
	bool isFreeHere(LPVOID addr) const;

	static const size_t NUMBER_OF_SIZE_TYPES = 3;
	static constexpr size_t SIZE_TYPES_LOWER_BOUNDS[NUMBER_OF_SIZE_TYPES] = { 0, 4 * 1024, 128 * 1024 };
	static const int PAGES_SUBSCRIBE = 1;
	static const int PAGES_UNSUBSCRIBE = -1;
	static const int PAGES_TRY_CLEAR = 0;
	_SYSTEM_INFO systemInfo;

	// ������ ������ ����
	LPVOID heap;

	// ������ ������� ��������� ������ ������ �������� �������
	std::set<Block, BlockComparator> freeBlocks[NUMBER_OF_SIZE_TYPES];
	std::map<LPVOID, size_t> freeAddresses;

	size_t heapSize;
	// ���������� ������ �� ������ �� �������
	std::vector<int> pages;

};
