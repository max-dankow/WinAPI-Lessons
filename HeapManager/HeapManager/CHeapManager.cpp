#include "CHeapManager.h"
#include <exception>
#include <cassert>

CHeapManager::CHeapManager() : 
	heap(0), 
	heapSize(0)
{
	GetSystemInfo(&systemInfo);
}

void CHeapManager::Create( size_t minSize, size_t maxSize )
{
	assert(minSize < maxSize);
	maxSize = round(maxSize, systemInfo.dwPageSize);
	minSize = round(minSize, systemInfo.dwPageSize);
	if(heap == 0) {
		// Резервируем память для кучи
		heap = VirtualAlloc(NULL, maxSize, MEM_RESERVE, PAGE_READWRITE );
		if (heap == 0) {
			throw std::bad_alloc();
		}
		// Предоставляет фрагменту minSize физическую память
		LPVOID commitResult = VirtualAlloc( heap, minSize, MEM_COMMIT, PAGE_READWRITE );
		if( commitResult == 0 ) {
			VirtualFree( heap, maxSize, MEM_RELEASE );
			heap = 0;
			throw std::bad_alloc();
		}
		heapSize = maxSize;
		initializePageUsageCounters();
		addFreeBlock(Block(heap, heapSize));
	} else {
		throw std::logic_error("Heap is already created");
	}
}

void* CHeapManager::Alloc( size_t size )
{
	size = round( size + sizeof(Heading), 4 );
	Block suitableBlock = findSuitableFreeBlock( size );
	if( suitableBlock.addr == 0 ) {
		std::cerr << "Can't allocate more" << std::endl;
		throw std::bad_alloc();
	}
	Block newBlock = biteOfNewBlock( suitableBlock, size );
	allocateBlock( newBlock );
	return getBlockBodyAddr( newBlock );
}

void CHeapManager::Free( void* mem )
{
	// Заголовок удаляемого блока
	Heading* heading = (Heading*)( static_cast<byte*>(mem) - sizeof(Heading) );
	Block currentBlock = Block(heading, heading->blockSize);
	Heading* left = heading->prev;
	Heading* right = (Heading*) getNext( currentBlock );
	size_t newTotalSize = heading->blockSize;
	LPVOID newFreeBlockAddr = heading;
	// Если слева примыкает свободный блок, то left не 0 по постронию
	if ( left != 0) {
		newFreeBlockAddr = left;
		newTotalSize += (byte*)heading - (byte*)left;
		removeFreeBlock(Block(left, (byte*)heading - (byte*)left));
	}
	if (isFreeHere(right)) {
        size_t sizeAtRight = freeAddresses.at(right);
		newTotalSize += sizeAtRight;
		removeFreeBlock(Block(right, sizeAtRight));
	}
	addFreeBlock(Block(newFreeBlockAddr, newTotalSize));
	updatePages(currentBlock, PAGES_UNSUBSCRIBE);
}

void CHeapManager::Destroy()
{
	LPVOID current = heap;
	while (current != 0) {
		if (!isFreeHere(current)) {
			Heading* heading = (Heading*)current;
			std::cout << "MEMORY LEAK: \t" << heading->blockSize - sizeof(Heading) << "\t at " << std::hex << heading << std::dec << std::endl;
			Free(static_cast<byte*>(current) + sizeof(Heading));
			current = heap;
		} else {
			current = getNext(Block(current, freeAddresses.at(current)));
		}
	}
}

void CHeapManager::Describe()
{
	std::cout << "Heap address: " << std::hex << heap << std::dec << std::endl;
	std::cout << "Heap size: " << heapSize << "(" << heapSize / systemInfo.dwPageSize << " pages)" << std::endl;

	for( size_t sizeType = 0; sizeType < NUMBER_OF_SIZE_TYPES; ++sizeType ) {
		for( Block freeBlock : freeBlocks[sizeType] ) {
			std::cout << "SIZE_TYPE " << sizeType << ": \t" << std::hex << freeBlock.addr << std::dec << '\t' << freeBlock.size << std::endl;
		}
	}
	for (auto pair : freeAddresses) {
		std::cout << "FREE AT " << " \t" << std::hex << pair.first << std::dec << '\t' << pair.second << std::endl;
	}

	for (int count : pages) {
		std::cout << count << ' ';
	}
	std::cout << std::endl;
	std::cout << std::endl;
}

// Округляет value до числа кратного roundTo
size_t CHeapManager::round( size_t value, size_t roundTo ) { 
	return ( value / roundTo ) * roundTo + ( ( value % roundTo == 0 ) ? 0 : roundTo );
}

size_t CHeapManager::getSizeType(size_t size)
{
	size_t sizeType = 0;
	for ( size_t type = 0; type < NUMBER_OF_SIZE_TYPES; ++type ) {
		if (SIZE_TYPES_LOWER_BOUNDS[type] <= size) {
			sizeType = type;
		}
	}
	return sizeType;
}

// Инициализирует массив с количеством блоков на каждой из страниц
void CHeapManager::initializePageUsageCounters()
{
	size_t numberOfPages = heapSize / systemInfo.dwPageSize;
	pages.assign( numberOfPages + 1, 0 );
}

// Проходит по списку блоков и находит подходящее место размера size
Block CHeapManager::findSuitableFreeBlock( size_t size )
{
    Block minimumBlock = Block(0, size);
	// Минимальный допустимый тип размера
	size_t minSizeType = getSizeType( size );
	// Если не найдем подходящего блока желаемого размера, придется искать среди более крупных
	for( size_t type = minSizeType; type < NUMBER_OF_SIZE_TYPES; ++type ) {
        auto candidate = freeBlocks[type].lower_bound(minimumBlock);
        if (candidate != freeBlocks[type].end()) {
            assert(candidate->size >= size);
            return *candidate;
        }
		//for (auto blockIter = freeBlocks[type].begin(); blockIter != freeBlocks[type].end(); ++blockIter) {
		//	if (blockIter->size >= size) {
		//		return *blockIter;
		//	}
		//}
	}
	return Block(0, 0);
}

// Обеспечивает блок закоммиченной памятью
void CHeapManager::ensureBlockIsCommitted(const Block block)
{
	// Закоммитятся не закоммиченные страницы
	LPVOID commitResult = VirtualAlloc(block.addr, block.size, MEM_COMMIT, PAGE_READWRITE);
	if( commitResult == 0 ) {
		throw std::bad_alloc();
	}
}

void CHeapManager::releasePage(LPVOID start, size_t size)
{
	VirtualFree(start, size, MEM_DECOMMIT);
}

void CHeapManager::updatePages(const Block block, int operation)
{
	assert(operation == PAGES_SUBSCRIBE || operation == PAGES_UNSUBSCRIBE);
	// Обновление pages
	size_t offset = static_cast<byte*>(block.addr) - static_cast<byte*>(heap);
	size_t startPage = offset / systemInfo.dwPageSize;
	size_t endPage = (offset + block.size) / systemInfo.dwPageSize;

    int startOfReleasing;
    bool isInReleasingFragment = false;
	for (size_t i = startPage; i <= endPage; ++i) {
		pages[i] += operation;
		if( pages[i] == 0 ) {
            if (!isInReleasingFragment) {
                startOfReleasing = i;
            }
            isInReleasingFragment = true;
        } else {
            if (isInReleasingFragment) {
                size_t size = (i - startOfReleasing) * systemInfo.dwPageSize;
                releasePage(static_cast<byte*>(heap) + startOfReleasing * systemInfo.dwPageSize, size);
            }
            isInReleasingFragment = false;
        }
	}
}

// Отделяем блок требуемого размера от свободного блока(отделенный блок не свободен, не аллоцирован)
Block CHeapManager::biteOfNewBlock(const Block source, size_t size)
{
	assert(source.size >= size);
	removeFreeBlock(source);
	// Если после отделения нового блока останется свободное место, то добавим еще свободный блок
	if( source.size > size + sizeof( Heading ) ) {
		LPVOID newBlockAddr = static_cast<byte*>( source.addr ) + size;
		size_t newSize = source.size - size;
		addFreeBlock( Block( newBlockAddr, newSize ) );
	}
	Block bittenBlock = Block( source.addr, size );
	return bittenBlock;
}

void CHeapManager::addFreeBlock( const Block block )
{
	freeBlocks[getSizeType(block.size)].insert(block);
	freeAddresses.insert(std::make_pair(block.addr, block.size));
	// Обновим у последующего аллоцированного блока указатель prev
	Heading* next = (Heading*) getNext(block);
	if( next != 0 ) {
		next->prev = (Heading*) block.addr;
	}
}

void CHeapManager::removeFreeBlock(const Block block)
{
	freeBlocks[getSizeType(block.size)].erase(block);
	freeAddresses.erase(block.addr);
}

void CHeapManager::allocateBlock( const Block block )
{
	ensureBlockIsCommitted( block );
	updatePages( block, PAGES_SUBSCRIBE );
	Heading* heading = getHeadingAddr( block );
	heading->blockSize = block.size;
	// Недопустима ситуация, когда перед block есть свободный блок, значит там либо занятый, либо ничего
	heading->prev = 0;
}

Heading* CHeapManager::getHeadingAddr( const Block block ) const
{
	return (Heading*) block.addr;
}

LPVOID CHeapManager::getBlockBodyAddr( const Block block ) const
{
	return static_cast<LPVOID>(static_cast<byte*>(block.addr) + sizeof(Heading));
}

// Возвращает блок, следующий непосредственно за указанным или нулевой блок, если указанный блок был последним
LPVOID CHeapManager::getNext( const Block block ) const
{
	// Если это последний блок, то следующего точно нет
	if (static_cast<byte*> (block.addr) - static_cast<byte*> (heap) + block.size + sizeof(Heading) <= heapSize ) {
		return static_cast<byte*> (block.addr) + block.size;
	} else {
		return 0;
	}
	
}

bool CHeapManager::isFreeHere( LPVOID addr ) const
{
	return addr != 0 &&  freeAddresses.find( addr ) != freeAddresses.end();
}