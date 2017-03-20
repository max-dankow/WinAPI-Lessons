#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
#include <set>

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
	size_t size;
};

struct BlockComparator {
	bool operator()(Block const & a, Block const & b) const
	{
		return a.addr < b.addr;
	}
};



class CHeapManager {

public:
	CHeapManager();
	~CHeapManager();

	// Резервирует непрерывный регион памяти размером maxSize, 
	// предоставляет блоку minSize физическую память
	void Create( size_t minSize, size_t maxSize );
	void* Alloc( size_t size );
	void Free( void* mem );
	void Describe();

private:
	static size_t round( size_t value, size_t roundTo );
	void initializePages();
	Block findSuitableFreeBlock(size_t size);
	void mergeNext(Block block);
	void ensureBlockIsCommitted(const Block block);
	void releasePage(LPVOID);
	void updatePages(const Block block, int sign);
	Block biteOf(Block source, size_t size);
	void addFreeBlock(const Block, Heading*);
	void allocateBlock(const Block);
	Heading* getHeadingAddr(const Block) const;
	LPVOID getBlockBodyAddr(const Block) const;
	Block getNext(const Block) const;

	static const int PAGES_SUBSCRIBE = 1;
	static const int PAGES_UNSUBSCRIBE = -1;
	static const int PAGES_TRY_CLEAR = 0;
	_SYSTEM_INFO systemInfo;
	LPVOID heap;
	std::set<Block, BlockComparator> freeSmall;
	std::set<Block, BlockComparator> allocated;
	size_t heapSize;
	// Количество блоков на каждой из страниц
	std::vector<int> pages;

};
