#include "CHeapManager.h"
#include <exception>
#include <cassert>

CHeapManager::CHeapManager() : 
	heap(0), 
	heapSize(0)
{
	GetSystemInfo(&systemInfo);
}

CHeapManager::~CHeapManager()
{
}

void CHeapManager::Create( size_t minSize, size_t maxSize )
{
	assert(minSize < maxSize);
	maxSize = round(maxSize, systemInfo.dwPageSize);
	minSize = round(minSize, systemInfo.dwPageSize);
	if(heap == 0) {
		// ����������� ������ ��� ����
		heap = VirtualAlloc(NULL, maxSize, MEM_RESERVE, PAGE_READWRITE );
		if (heap == 0) {
			throw std::bad_alloc();
		}
		// ������������� ��������� minSize ���������� ������
		LPVOID commitResult = VirtualAlloc( heap, minSize, MEM_COMMIT, PAGE_READWRITE );
		if( commitResult == 0 ) {
			VirtualFree( heap, maxSize, MEM_RELEASE );
			heap = 0;
			throw std::bad_alloc();
		}
		heapSize = maxSize;
		initializePages();
		addFreeBlock(Block(heap, heapSize), Heading(0, 0));
	} else {
		throw std::logic_error("Heap is already created");
	}
}

void* CHeapManager::Alloc( size_t size )
{
	size = round( size, 4 );
	Block suitableBlock = findSuitableFreeBlock( size );
	if (suitableBlock.addr == 0) {
		std::cerr << "Can't allocate more" << std::endl;
		exit(EXIT_FAILURE);
	}
	Block newBlock = biteOf(suitableBlock, size);
	ensureBlockIsCommitted(newBlock);
	updatePages(newBlock, +1);
	freeSmall.erase(newBlock);
	allocated.insert(newBlock);
	 
	return getBody(newBlock);
}

void CHeapManager::Free(void * mem)
{
}

// ��������� value �� ����� �������� roundTo
size_t CHeapManager::round( size_t value, size_t roundTo ) { 
	return ( value / roundTo ) * roundTo + ( ( value % roundTo == 0 ) ? 0 : roundTo );
}

// �������������� ������ � ����������� ������ �� ������ �� �������
void CHeapManager::initializePages()
{
	size_t numberOfPages = heapSize / systemInfo.dwPageSize;
	pages.assign( numberOfPages + 1, 0 );
}

// �������� �� ������ ������ � ������� ���������� ����� ������� size
Block CHeapManager::findSuitableFreeBlock( size_t size )
{
	for (auto blockIter = freeSmall.begin(); blockIter != freeSmall.end(); ++blockIter) {
		if ( blockIter->size >= size ) {
			return *blockIter;
		}
	}
	return Block(0, 0);
	/*Heading* suitableBlock = 0;
	Heading* currentBlock = (Heading*) heap;
	while ( currentBlock != 0 ) {
		if ( currentBlock->isFree && currentBlock->blockSize >= size ) {
			suitableBlock = currentBlock;
			break;
		}
		currentBlock = currentBlock->next;
	}
	return suitableBlock;*/
}

void CHeapManager::mergeNext(Heading *block)
{
	/*if( block->next != NULL && block->next->isFree ) {
		block->blockSize += block->next->blockSize + sizeof(Heading);

		if( block->next->next != NULL ) {
			block->next->next->prev = block;
		}

		block->next = block->next->next;
	}*/
}

// ������������ ���� ������������� �������, ��������� pages
void CHeapManager::ensureBlockIsCommitted(const Block block)
{
	// ������������ �� ������������� ��������
	LPVOID commitResult = VirtualAlloc(block.addr, block.size, MEM_COMMIT, PAGE_READWRITE);
	if( commitResult == 0 ) {
		throw std::bad_alloc();
	}
}

// todo: simplify
void CHeapManager::updatePages(const Block block, int sign)
{
	assert(sign == 1 || sign == -1);
	// ���������� pages
	size_t offset = static_cast<byte*>(block.addr) - static_cast<byte*>(heap);
	size_t startPage = offset / systemInfo.dwPageSize;
	size_t endPage = (offset + block.size + sizeof(Heading)) / systemInfo.dwPageSize;
	for (size_t i = startPage; i <= endPage; ++i) {
		pages[i] += sign;
	}
}

// �������� ���� ���������� ������� �� ���������� �����
Block CHeapManager::biteOf(Block source, size_t size)
{
	assert(source.size >= size);
	// ���� ��� ��������� ��������� �����, �� ������� ��������� ����
	if( source.size > size + sizeof( Heading ) ) {
		LPVOID newBlockAddr = static_cast<byte*>( source.addr ) + size + sizeof(Heading);
		size_t newSize = source.size - size - sizeof(Heading);
		Heading newHeading = Heading(getHeadingAddr(source), newSize);
		addFreeBlock( Block( newBlockAddr, newSize), newHeading);
	}
	freeSmall.erase(source);
	// � ��������� ����� ������ �������� ��� ����������
	updatePages(Block(source.addr, sizeof(Heading)), -1);
	Block bittenBlock = Block(source.addr, size);
	freeSmall.insert(bittenBlock);
	return bittenBlock;
}

void CHeapManager::addFreeBlock(const Block block, const Heading heading)
{
	// ������������ �� ������������� ��������
	ensureBlockIsCommitted(Block(block.addr, sizeof(Heading)));
	updatePages(Block(block.addr, sizeof(Heading)), 1);
	freeSmall.insert(block);
	Heading* headingAddr = (Heading*)block.addr;
	Block next = getNext(block);
	if (next.addr != 0) {
		getHeadingAddr(next)->prev = getHeadingAddr(block);
	}
}

Heading* CHeapManager::getHeadingAddr( const Block block ) const
{
	return (Heading*) block.addr;
}

LPVOID CHeapManager::getBody(const Block block) const
{
	return static_cast<LPVOID>(static_cast<byte*>(block.addr) + sizeof(Heading));
}

Block CHeapManager::getNext(const Block block) const
{
	// ���� ��� ��������� ����, �� ���������� ����� ���
	if( static_cast<byte*> (block.addr) - static_cast<byte*> (heap) + block.size + sizeof(Heading) <= heapSize ) {
		Heading* nextHeading = (Heading*)(static_cast<byte*> (block.addr) + block.size + sizeof(Heading));
		return Block(nextHeading, nextHeading->blockSize);
	} else {
		return Block(0, 0);
	}
	
}
