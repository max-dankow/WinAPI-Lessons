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
		addFreeBlock(Block(heap, heapSize), 0);
	} else {
		throw std::logic_error("Heap is already created");
	}
}

void* CHeapManager::Alloc( size_t size )
{
	size = round( size, 4 );
	Block suitableBlock = findSuitableFreeBlock( size );
	if( suitableBlock.addr == 0 ) {
		std::cerr << "Can't allocate more" << std::endl;
		exit(EXIT_FAILURE);
	}
	Block newBlock = biteOf(suitableBlock, size);
	allocateBlock(newBlock);
	return getBlockBodyAddr(newBlock);
}

void CHeapManager::Free( void* mem )
{
	
}

void CHeapManager::mergeNext( Block block )
{
}

void CHeapManager::Describe()
{
	std::cout << "Heap address: " << std::hex << heap << std::dec << std::endl;
	std::cout << "Heap size: " << heapSize << "(" << heapSize / systemInfo.dwPageSize << " pages)" << std::endl;

	for( Block freeBlock : freeSmall ) {
		std::cout << "Small free block: \t" << std::hex << freeBlock.addr << std::dec << '\t' << freeBlock.size << std::endl;
	}

	for (Block allocatedBlock : allocated) {
		std::cout << "Allocated block: \t" << std::hex << allocatedBlock.addr << std::dec << '\t' << allocatedBlock.size << std::endl;
	}

	std::cout << std::endl;
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
}

// ������������ ���� ������������� �������
void CHeapManager::ensureBlockIsCommitted(const Block block)
{
	// ������������ �� ������������� ��������
	LPVOID commitResult = VirtualAlloc(block.addr, sizeof(Heading) + block.size, MEM_COMMIT, PAGE_READWRITE);
	if( commitResult == 0 ) {
		throw std::bad_alloc();
	}
}

void CHeapManager::releasePage(LPVOID page)
{
	VirtualFree(page, systemInfo.dwPageSize, MEM_DECOMMIT);
}

void CHeapManager::updatePages(const Block block, int operation)
{
	assert(operation == PAGES_SUBSCRIBE || operation == PAGES_UNSUBSCRIBE || operation == PAGES_TRY_CLEAR);
	// ���������� pages
	size_t offset = static_cast<byte*>(block.addr) - static_cast<byte*>(heap);
	size_t startPage = offset / systemInfo.dwPageSize;
	size_t endPage = (offset + block.size + sizeof(Heading)) / systemInfo.dwPageSize;
	for (size_t i = startPage; i <= endPage; ++i) {
		if( operation == 0 && pages[i] == 0) {
			releasePage(static_cast<byte*>(heap) + i * systemInfo.dwPageSize);
		} else {
			pages[i] += operation;
		}
	}
}

// �������� ���� ���������� ������� �� ���������� �����
Block CHeapManager::biteOf(Block source, size_t size)
{
	assert(source.size >= size);
	Heading* sourceHeading = getHeadingAddr(source);
	// ���� ��� ��������� ��������� �����, �� ������� ��������� ����
	if( source.size > size + sizeof( Heading ) ) {
		LPVOID newBlockAddr = static_cast<byte*>( source.addr ) + sizeof(Heading) + size;
		size_t newSize = source.size - size - sizeof(Heading);
		addFreeBlock( Block( newBlockAddr, newSize ), sourceHeading );
	}
	Block bittenBlock = Block(source.addr, size);

	freeSmall.erase( source );
	freeSmall.insert( bittenBlock );
	return bittenBlock;
}

// ��������� ��������� ����, ����������� ��� Heading, 
// ��������� ��������� �� ���������� ���� � ��������� ��������.
// ����� ������ ������ ��������.
void CHeapManager::addFreeBlock(const Block block, Heading* previous)
{
	// ������ ��� ��������� ������ ���� �����������
	ensureBlockIsCommitted(Block(block.addr, 0));
	updatePages(Block(block.addr, 0), PAGES_SUBSCRIBE);
	freeSmall.insert(block);
	// ����������� ���������
	Heading* headingAddr = (Heading*)block.addr;
	headingAddr->blockSize = block.size;
	headingAddr->prev = previous;
	Block next = getNext(block);
	if (next.addr != 0) {
		getHeadingAddr(next)->prev = getHeadingAddr(block);
	}
}

void CHeapManager::allocateBlock( const Block block )
{
	// � ��������� ����� ������ ��������� ��� ����������
	updatePages( Block(block.addr, 0), PAGES_UNSUBSCRIBE);
	freeSmall.erase( block );

	ensureBlockIsCommitted( block );
	updatePages( block, PAGES_SUBSCRIBE);
	allocated.insert( block );
}

Heading* CHeapManager::getHeadingAddr( const Block block ) const
{
	return (Heading*) block.addr;
}

LPVOID CHeapManager::getBlockBodyAddr( const Block block ) const
{
	return static_cast<LPVOID>(static_cast<byte*>(block.addr) + sizeof(Heading));
}

// ���������� ����, ��������� ��������������� �� ��������� ��� ������� ����, ���� ��������� ���� ��� ���������
Block CHeapManager::getNext(const Block block) const
{
	// ���� ��� ��������� ����, �� ���������� ����� ���
	if (static_cast<byte*> (block.addr) - static_cast<byte*> (heap) + sizeof(Heading) + block.size + sizeof(Heading) <= heapSize ) {
		Heading* nextHeading = (Heading*)(static_cast<byte*> (block.addr) + sizeof(Heading) + block.size);
		return Block(nextHeading, nextHeading->blockSize);
	} else {
		return Block(0, 0);
	}
	
}
