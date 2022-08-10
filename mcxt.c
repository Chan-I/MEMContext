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