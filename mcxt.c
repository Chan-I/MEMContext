#include "mcxt.h"

static AllocSetFreeList context_freelists[2] = {{0, NULL}, {0, NULL}};
/*
 *	 name:	MemoryContextCreate
 * 	 function:	初始化内存上下文结构
 *	 params：	node    当前的节点
 *              parent	父节点
 *				name	内存上下文名称
 */
void MemoryContextCreate(MemoryContext node,
                         MemoryContext parent,
                         const char *name)
{
    /* Initialize all standard fields of memory context header */
    node->isReset = 1;
    //   node->methods = methods;
    node->parent = parent;
    node->first_child = NULL;
    node->next_child = NULL;
    node->prev_child = NULL;
    node->name = (char *)name;

    /* OK to link node into context tree */
    if (parent)
    {
        node->next_child = parent->first_child;
        if (parent->first_child != NULL)
            parent->first_child->prev_child = node;
        parent->first_child = node;
    }
    else
    {
        node->next_child = NULL;
    }
}

/*
 *	 name:	AllocSetContextCreateInternal
 * 	 function:	初始化AllocSet结构（上下文结构遵循MemoryContext）
 *	 params：
 *              parent	父节点所属的上下文节点
 *				name	内存上下文名称，便于debug
 *              minContextSize  内存申请最小值
 *              initBlockSize   初始化申请值
 *              maxBlockSize    最大申请值
 */
MemoryContext
AllocSetContextCreateInternal(MemoryContext parent,
                              const char *name,
                              Size minContextSize,
                              Size initBlockSize,
                              Size maxBlockSize)
{
    int freeListIndex;
    Size firstBlockSize;
    AllocSet set;
    AllocBlock block;

    /**
     * @brief 首先检查参数是否满足某个可用的freelist，
     *        不必一定要满足maxBlockSize。
     *
     */
    if (minContextSize == ALLOCSET_DEFAULT_MINSIZE &&
        initBlockSize == ALLOCSET_DEFAULT_INITSIZE &&
        maxBlockSize == ALLOCSET_DEFAULT_MAXSIZE)
    {
        freeListIndex = 0;
    }
    else if (minContextSize == ALLOCSET_DEFAULT_MINSIZE &&
             initBlockSize == ALLOCSET_DEFAULT_INITSIZE)
    {
        freeListIndex = 1;
    }
    else
    {
        freeListIndex = -1;
    }

    /* 如果存在合适的freelist，则直接回收对应的context */
    if (freeListIndex >= 0)
    {
        AllocSetFreeList *freelist = &context_freelists[freeListIndex];

        if (freelist->first_free != NULL)
        {
            set = freelist->first_free;
            freelist->first_free = (AllocSet)set->header.next_child;
            freelist->num_free--;

            set->maxBlockSize = maxBlockSize;

            MemoryContextCreate((MemoryContext)set,
                                parent,
                                name);

            return (MemoryContext)set;
        }
    }

    /* 判断init的block大小 */
    firstBlockSize = MAXALIGN(sizeof(AllocSetContext)) +
                     ALLOC_BLOCKHDRSZ + ALLOC_CHUNKHDRSZ;
    if (minContextSize != 0)
        firstBlockSize = Max(firstBlockSize, minContextSize);
    else
        firstBlockSize = Max(firstBlockSize, initBlockSize);

    set = (AllocSet)malloc(firstBlockSize);
    if (set == NULL)
    {
        fprintf(stderr, "out of memory, Failed while creating memory context.");
        exit(0);
    }

    block = (AllocBlock)(((char *)set) + MAXALIGN(sizeof(AllocSetContext)));
    block->aset = set;
    block->freeptr = ((char *)block) + ALLOC_BLOCKHDRSZ;
    block->endptr = ((char *)set) + firstBlockSize;
    block->prev = NULL;
    block->next = NULL;

    set->blocks = block;
    set->keeper = block;

    MemSetAligned(set->freelist, 0, sizeof(set->freelist));

    set->initBlockSize = initBlockSize;
    set->maxBlockSize = maxBlockSize;
    set->nextBlockSize = initBlockSize;
    set->freeListIndex = freeListIndex;
    set->allocChunkLimit = ALLOC_CHUNK_LIMIT;

    while ((Size)(set->allocChunkLimit + ALLOC_CHUNKHDRSZ) >
           (Size)((maxBlockSize - ALLOC_BLOCKHDRSZ) / ALLOC_CHUNK_FRACTION))
        set->allocChunkLimit >>= 1;

    MemoryContextCreate((MemoryContext)set,
                        parent,
                        name);

    return (MemoryContext)set;
}

static inline int
AllocSetFreeIndex(Size size)
{
    int idx;
    unsigned int t,
        tsize;

    if (size > (1 << ALLOC_MINBITS))
    {
        tsize = (size - 1) >> ALLOC_MINBITS;
        t = tsize >> 8;
        idx = t ? LogTable256[t] + 8 : LogTable256[t];
    }
    else
    {
        idx = 0;
    }
    return idx;
}

static void *
AllocSetAlloc(MemoryContext context, Size size)
{
    AllocSet set = (AllocSet)context;
    AllocBlock block;
    AllocChunk chunk;
    int fidx;
    Size chunk_size;
    Size blksize;

    /**
     * @brief 如果需要申请的空间大于分片要求的最大要求，
     *          则单独申请一个单独的内存块。
     */
    if (size > set->allocChunkLimit)
    {
        chunk_size = MAXALIGN(size);
        blksize = chunk_size + ALLOC_BLOCKHDRSZ + ALLOC_CHUNKHDRSZ;
        block = (AllocBlock)malloc(blksize);

        if (block == NULL)
            return NULL;

        block->aset = set;
        block->freeptr = block->endptr = ((char *)block) + blksize;

        chunk = (AllocChunk)(((char *)block) + ALLOC_BLOCKHDRSZ);
        chunk->aset = set;
        chunk->size = chunk_size;

        if (set->blocks != NULL)
        {
            block->prev = set->blocks;
            block->next = set->blocks->next;

            if (block->next)
                block->next->prev = block;

            set->blocks->next = block;
        }
        else
        {
            block->prev = NULL;
            block->next = NULL;
            set->blocks = block;
        }

        return AllocChunkGetPointer(chunk);
    }

    /**
     *      如果请求足够小，小到一个内存chunk就可以满足需求。
     *      查看响应的freelist，查看是否有可以重用的空闲chunk，
     *      如果找到，则将其从空闲列表中删除，使其再次称为
     *      分配的成员并返回他的数据地址。
     */
    fidx = AllocSetFreeIndex(size);
    chunk = set->freelist[fidx];
    if (chunk != NULL)
    {
        set->freelist[fidx] = (AllocChunk)chunk->aset;
        chunk->aset = (void *)set;

        return AllocChunkGetPointer(chunk);
    }

    chunk_size = (1 << ALLOC_MINBITS) << fidx;

    /**
     *    如果当前block有足够的可用空间，就可以将chunk放进此Block中，
     *    否则必须开辟新的block.
     */
    if ((block = set->blocks) != NULL)
    {
        Size availspace = block->endptr - block->freeptr;
        if (availspace < (chunk_size + ALLOC_CHUNKHDRSZ))
        {
            /**
             *   顶部的block没有足够空间给申请的chunk请求了，
             *   但是block中残留的空间可能还是足够其他申请使用的，
             *   一旦跳转到下一个block，其剩余的空间将被白白浪费。
             *   在此之前可以将其中剩余的空间一一瓜分，交给freelist来管理。
             *
             *   考虑到空间剩余，while循环不可能循环满整个freelist的11次。
             */

            while (availspace >= ((1 << ALLOC_MINBITS) + ALLOC_CHUNKHDRSZ))
            {
                Size availchunk = availspace - ALLOC_CHUNKHDRSZ;
                int a_fidx = AllocSetFreeIndex(availchunk);

                if (availchunk != ((Size)1 << (a_fidx + ALLOC_MINBITS)))
                {
                    a_fidx--;
                    availchunk = ((Size)1 << (a_fidx + ALLOC_MINBITS));
                }

                chunk = (AllocChunk)(block->freeptr);

                block->freeptr += (availchunk + ALLOC_CHUNKHDRSZ);
                availspace -= (availchunk + ALLOC_CHUNKHDRSZ);

                chunk->size = availchunk;
                chunk->aset = (void *)set->freelist[a_fidx];
                set->freelist[a_fidx] = chunk;
            }
            /* 表名需要开辟一个新的block了 */
            block = NULL;
        }
    }

    if (block == NULL)
    {
        Size required_size;

        blksize = set->nextBlockSize;
        set->nextBlockSize <<= 1;
        if (set->nextBlockSize > set->maxBlockSize)
        {
            set->nextBlockSize = set->maxBlockSize;
        }

        /**
         *   if initBlockSize is not big enough,(< ALLOC_CHUNK_LIMIT)
         *    double blksize for times until it is big enough.
         */
        required_size = chunk_size + ALLOC_BLOCKHDRSZ + ALLOC_CHUNKHDRSZ;
        while (blksize < required_size)
            blksize <<= 1;

        block = (AllocBlock)malloc(blksize);

        while (block == NULL && blksize > 1024 * 1024)
        {
            blksize >>= 1;
            if (blksize < required_size)
                break;

            block = (AllocBlock)malloc(blksize);
        }
        if (block == NULL)
            return NULL;

        block->aset = set;
        block->freeptr = ((char *)block) + ALLOC_BLOCKHDRSZ;
        block->endptr = ((char *)block) + blksize;

        block->prev = NULL;
        block->next = set->blocks;
        if (block->next)
            block->next->prev = block;
        set->blocks = block;
    }

    chunk = (AllocChunk)(block->freeptr);

    block->freeptr += (chunk_size + ALLOC_CHUNKHDRSZ);
    chunk->aset = (void *)set;
    chunk->size = chunk_size;

    return AllocChunkGetPointer(chunk);
}