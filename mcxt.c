#include "mcxt.h"

#define Max(_x, _y) ((_x) > (_y) ? (_x) : (_y))

MemoryContext
MemoryContextSwitchTo(MemoryContext context)
{
    MemoryContext old = CurrentMemoryContext;

    CurrentMemoryContext = context;
    return old;
}

void MemoryContextInit(void)
{
    TopMemoryContext = AllocSetContextCreate(NULL,
                                             "TopMemoryContext",
                                             ALLOCSET_DEFAULT_SIZES);
    CurrentMemoryContext = TopMemoryContext;

    ErrorMemoryContext = AllocSetContextCreate(TopMemoryContext,
                                               "ErrorMemoryContext",
                                               8 * 1024,
                                               8 * 1024,
                                               8 * 1024);
}

void *
repalloc(void *pointer, Size size)
{
    MemoryContext context = GetMemoryChunkContext(pointer);
    void *ret;

    if (!((Size)(size) <= (Size)0x3fffffff))
        fprintf(stderr, "invalid memory alloc request size %zu", size);

    ret = AllocSetRealloc(context, pointer, size);
    if ((ret == NULL) != 0)
    {
        fprintf(stderr, "Failed on request of size %zu in memory context \"%s\".",
                size, context->name);
    }

    return ret;
}

void *
palloc(Size size)
{
    /* duplicates MemoryContextAlloc to avoid increased overhead */
    void *ret;
    MemoryContext context = CurrentMemoryContext;

    if (!((Size)(size) <= ((Size)0x3fffffff)))
        fprintf(stderr, "invalid memory alloc request size %zu", size);

    context->isReset = 0;

    ret = AllocSetAlloc(context, size);
    if ((ret == NULL) != 0)
    {
        fprintf(stderr, "Failed on request of size %zu in memory context \"%s\".",
                size, context->name);
        exit(0);
    }

    return ret;
}

void pfree(void *pointer)
{
    MemoryContext context = GetMemoryChunkContext(pointer);

    AllocSetFree(context, pointer);
}

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

int AllocSetFreeIndex(Size size)
{
    int idx;
    unsigned int t,
        tsize;

    if (size > (1 << ALLOC_MINBITS))
    {
        tsize = (size - 1) >> ALLOC_MINBITS;
        t = tsize >> 8;
        idx = t ? LogTable256[t] + 8 : LogTable256[tsize];
    }
    else
    {
        idx = 0;
    }
    return idx;
}

void *
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

void AllocSetFree(MemoryContext context, void *pointer)
{
    AllocSet set = (AllocSet)context;
    AllocChunk chunk = AllocPointerGetChunk(pointer);

    if (chunk->size > set->allocChunkLimit)
    {
        /*
         * Big chunks are certain to have been allocated as single-chunk
         * blocks.  Just unlink that block and return it to malloc().
         */
        AllocBlock block = (AllocBlock)(((char *)chunk) - ALLOC_BLOCKHDRSZ);

        /*
         * Try to verify that we have a sane block pointer: it should
         * reference the correct aset, and freeptr and endptr should point
         * just past the chunk.
         */
        if (block->aset != set ||
            block->freeptr != block->endptr ||
            block->freeptr != ((char *)block) +
                                  (chunk->size + ALLOC_BLOCKHDRSZ + ALLOC_CHUNKHDRSZ))
        {
            fprintf(stderr, "could not find block containing chunk %p", chunk);
            exit(0);
        }
        /* OK, remove block from aset's list and free it */
        if (block->prev)
            block->prev->next = block->next;
        else
            set->blocks = block->next;
        if (block->next)
            block->next->prev = block->prev;

        free(block);
    }
    else
    {
        /* Normal case, put the chunk into appropriate freelist */
        int fidx = AllocSetFreeIndex(chunk->size);
        chunk->aset = (void *)set->freelist[fidx];
        set->freelist[fidx] = chunk;
    }
}

/*
 * AllocSetRealloc
 *		Returns new pointer to allocated memory of given size or NULL if
 *		request could not be completed; this memory is added to the set.
 *		Memory associated with given pointer is copied into the new memory,
 *		and the old memory is freed.
 *
 * Without MEMORY_CONTEXT_CHECKING, we don't know the old request size.  This
 * makes our Valgrind client requests less-precise, hazarding false negatives.
 * (In principle, we could use VALGRIND_GET_VBITS() to rediscover the old
 * request size.)
 */
void *
AllocSetRealloc(MemoryContext context, void *pointer, Size size)
{
    AllocSet set = (AllocSet)context;
    AllocChunk chunk = AllocPointerGetChunk(pointer);
    Size oldsize;

    oldsize = chunk->size;

    if (oldsize > set->allocChunkLimit)
    {
        /*
         * The chunk must have been allocated as a single-chunk block.  Use
         * realloc() to make the containing block bigger, or smaller, with
         * minimum space wastage.
         */
        AllocBlock block = (AllocBlock)(((char *)chunk) - ALLOC_BLOCKHDRSZ);
        Size chksize;
        Size blksize;
        Size oldblksize;

        /*
         * Try to verify that we have a sane block pointer: it should
         * reference the correct aset, and freeptr and endptr should point
         * just past the chunk.
         */
        if (block->aset != set ||
            block->freeptr != block->endptr ||
            block->freeptr != ((char *)block) +
                                  (oldsize + ALLOC_BLOCKHDRSZ + ALLOC_CHUNKHDRSZ))
        {
            fprintf(stderr, "could not find block containing chunk %p", chunk);
            exit(0);
        }
        /*
         * Even if the new request is less than set->allocChunkLimit, we stick
         * with the single-chunk block approach.  Therefore we need
         * chunk->size to be bigger than set->allocChunkLimit, so we don't get
         * confused about the chunk's status in future calls.
         */
        chksize = Max(size, set->allocChunkLimit + 1);
        chksize = MAXALIGN(chksize);

        /* Do the realloc */
        blksize = chksize + ALLOC_BLOCKHDRSZ + ALLOC_CHUNKHDRSZ;
        oldblksize = block->endptr - ((char *)block);

        block = (AllocBlock)realloc(block, blksize);
        if (block == NULL)
            return NULL;

        /* updated separately, not to underflow when (oldblksize > blksize) */
        block->freeptr = block->endptr = ((char *)block) + blksize;

        /* Update pointers since block has likely been moved */
        chunk = (AllocChunk)(((char *)block) + ALLOC_BLOCKHDRSZ);
        pointer = AllocChunkGetPointer(chunk);
        if (block->prev)
            block->prev->next = block;
        else
            set->blocks = block;
        if (block->next)
            block->next->prev = block;
        chunk->size = chksize;

        return pointer;
    }

    /*
     * Chunk sizes are aligned to power of 2 in AllocSetAlloc().  Maybe the
     * allocated area already is >= the new size.  (In particular, we will
     * fall out here if the requested size is a decrease.)
     */
    else if (oldsize >= size)
    {
        return pointer;
    }
    else
    {
        /*
         * Enlarge-a-small-chunk case.  We just do this by brute force, ie,
         * allocate a new chunk and copy the data.  Since we know the existing
         * data isn't huge, this won't involve any great memcpy expense, so
         * it's not worth being smarter.  (At one time we tried to avoid
         * memcpy when it was possible to enlarge the chunk in-place, but that
         * turns out to misbehave unpleasantly for repeated cycles of
         * palloc/repalloc/pfree: the eventually freed chunks go into the
         * wrong freelist for the next initial palloc request, and so we leak
         * memory indefinitely.  See pgsql-hackers archives for 2007-08-11.)
         */
        AllocPointer newPointer;

        /* allocate new chunk */
        newPointer = AllocSetAlloc((MemoryContext)set, size);

        /* leave immediately if request was not completed */
        if (newPointer == NULL)
        {
            /* Disallow external access to private part of chunk header. */
            return NULL;
        }

        /* transfer existing data (certain to fit) */
        memcpy(newPointer, pointer, oldsize);

        /* free old chunk */
        AllocSetFree((MemoryContext)set, pointer);

        return newPointer;
    }
}

/*
 * AllocSetDelete
 *		Frees all memory which is allocated in the given set,
 *		in preparation for deletion of the set.
 *
 * Unlike AllocSetReset, this *must* free all resources of the set.
 */
void AllocSetDelete(MemoryContext context)
{
    AllocSet set = (AllocSet)context;
    AllocBlock block = set->blocks;

    /*
     * If the context is a candidate for a freelist, put it into that freelist
     * instead of destroying it.
     */
    if (set->freeListIndex >= 0)
    {
        AllocSetFreeList *freelist = &context_freelists[set->freeListIndex];

        /*
         * Reset the context, if it needs it, so that we aren't hanging on to
         * more than the initial malloc chunk.
         */
        if (!context->isReset)
            MemoryContextResetOnly(context);

        /*
         * If the freelist is full, just discard what's already in it.  See
         * comments with context_freelists[].
         */
        if (freelist->num_free >= MAX_FREE_CONTEXTS)
        {
            while (freelist->first_free != NULL)
            {
                AllocSetContext *oldset = freelist->first_free;

                freelist->first_free = (AllocSetContext *)oldset->header.next_child;
                freelist->num_free--;

                /* All that remains is to free the header/initial block */
                free(oldset);
            }
        }

        /* Now add the just-deleted context to the freelist. */
        set->header.next_child = (MemoryContext)freelist->first_free;
        freelist->first_free = set;
        freelist->num_free++;

        return;
    }

    /* Free all blocks, except the keeper which is part of context header */
    while (block != NULL)
    {
        AllocBlock next = block->next;

        if (block != set->keeper)
            free(block);

        block = next;
    }

    /* Finally, free the context header, including the keeper block */
    free(set);
}

void AllocSetReset(MemoryContext context)
{
    AllocSet set = (AllocSet)context;
    AllocBlock block;

    /* Clear chunk freelists */
    MemSetAligned(set->freelist, 0, sizeof(set->freelist));
    block = set->blocks;

    /* New blocks list will be just the keeper block */
    set->blocks = set->keeper;

    while (block != NULL)
    {
        AllocBlock next = block->next;

        if (block == set->keeper)
        {
            /* Reset the block, but don't return it to malloc */
            char *datastart = ((char *)block) + ALLOC_BLOCKHDRSZ;
            block->freeptr = datastart;
            block->prev = NULL;
            block->next = NULL;
        }
        else
        {
            /* Normal case, release the block */
            free(block);
        }
        block = next;
    }

    /* Reset block size allocation sequence, too */
    set->nextBlockSize = set->initBlockSize;
}

void MemoryContextResetOnly(MemoryContext context)
{
    /* Nothing to do if no pallocs since startup or last reset */
    if (!context->isReset)
    {
        MemoryContextCallResetCallbacks(context);

        /*
         * If context->ident points into the context's memory, it will become
         * a dangling pointer.  We could prevent that by setting it to NULL
         * here, but that would break valid coding patterns that keep the
         * ident elsewhere, e.g. in a parent context.  Another idea is to use
         * MemoryContextContains(), but we don't require ident strings to be
         * in separately-palloc'd chunks, so that risks false positives.  So
         * for now we assume the programmer got it right.
         */

        AllocSetReset(context);
        context->isReset = 1;
    }
}

void MemoryContextCallResetCallbacks(MemoryContext context)
{
    MemoryContextCallback *cb;

    /*
     * We pop each callback from the list before calling.  That way, if an
     * error occurs inside the callback, we won't try to call it a second time
     * in the likely event that we reset or delete the context later.
     */
    while ((cb = context->reset_cbs) != NULL)
    {
        context->reset_cbs = cb->next;
        cb->func(cb->arg);
    }
}

void MemoryContextReset(MemoryContext context)
{
    /* save a function call in common case where there are no children */
    if (context->first_child != NULL)
        MemoryContextDeleteChildren(context);

    /* save a function call if no pallocs since startup or last reset */
    if (!context->isReset)
        MemoryContextResetOnly(context);
}

void MemoryContextDelete(MemoryContext context)
{
    /* save a function call in common case where there are no children */
    if (context->first_child != NULL)
        MemoryContextDeleteChildren(context);

    /*
     * It's not entirely clear whether 'tis better to do this before or after
     * delinking the context; but an error in a callback will likely result in
     * leaking the whole context (if it's not a root context) if we do it
     * after, so let's do it before.
     */
    MemoryContextCallResetCallbacks(context);

    /*
     * We delink the context from its parent before deleting it, so that if
     * there's an error we won't have deleted/busted contexts still attached
     * to the context tree.  Better a leak than a crash.
     */
    MemoryContextSetParent(context, NULL);

    /*
     * Also reset the context's ident pointer, in case it points into the
     * context.  This would only matter if someone tries to get stats on the
     * (already unlinked) context, which is unlikely, but let's be safe.
     */

    AllocSetDelete(context);
}

void MemoryContextDeleteChildren(MemoryContext context)
{
    /*
     * MemoryContextDelete will delink the child from me, so just iterate as
     * long as there is a child.
     */
    while (context->first_child != NULL)
        MemoryContextDelete(context->first_child);
}

void MemoryContextSetParent(MemoryContext context, MemoryContext new_parent)
{
    /* Fast path if it's got correct parent already */
    if (new_parent == context->parent)
        return;

    /* Delink from existing parent, if any */
    if (context->parent)
    {
        MemoryContext parent = context->parent;

        if (context->prev_child != NULL)
            context->prev_child->next_child = context->next_child;
        else
            parent->first_child = context->next_child;

        if (context->next_child != NULL)
            context->next_child->prev_child = context->prev_child;
    }

    /* And relink */
    if (new_parent)
    {
        context->parent = new_parent;
        context->prev_child = NULL;
        context->next_child = new_parent->first_child;
        if (new_parent->first_child != NULL)
            new_parent->first_child->prev_child = context;
        new_parent->first_child = context;
    }
    else
    {
        context->parent = NULL;
        context->prev_child = NULL;
        context->next_child = NULL;
    }
}

MemoryContext
GetMemoryChunkContext(void *pointer)
{
    MemoryContext context;

    /*
     * Try to detect bogus pointers handed to us, poorly though we can.
     * Presumably, a pointer that isn't MAXALIGNED isn't pointing at an
     * allocated chunk.
     */

    /*
     * OK, it's probably safe to look at the context.
     */
    context = *(MemoryContext *)(((char *)pointer) - sizeof(void *));

    return context;
}

extern StringInfo makeStringInfo(void)
{
    StringInfo res;
    res = (StringInfo)palloc(sizeof(StringInfoData));

    initStringInfo(res);

    return res;
}

extern void initStringInfo(StringInfo str)
{
    int size = 1024;

    str->data = (char *)palloc(size);
    str->maxlen = size;
    resetStringInfo(str);
}

extern void resetStringInfo(StringInfo str)
{
    str->data[0] = '\0';
    str->len = 0;
    str->cursor = 0;
}

void appendStringInfo(StringInfo str, const char *fmt, ...)
{
    int save_errno = errno;

    for (;;)
    {
        va_list args;
        int needed;

        errno = save_errno;
        va_start(args, fmt);
        needed = appendStringInfoVA(str, fmt, args);
        va_end(args);

        if (needed == 0)
            break;

        enlargeStringInfo(str, needed);
    }
}

int appendStringInfoVA(StringInfo str, const char *fmt, va_list args)
{
    int avail;
    Size nprinted;

    avail = str->maxlen - str->len;
    if (avail < 16)
        return 32;

    nprinted = vsnprintf(str->data + str->len, (Size)avail, fmt, args);
    if (nprinted < (Size)avail)
    {
        str->len += (int)nprinted;
        return 0;
    }

    str->data[str->len] = '\0';

    return (int)nprinted;
}

void enlargeStringInfo(StringInfo str, int needed)
{
    int newlen;
    int MaxAllocSize = (Size)0x3fffffff;

    if (needed < 0)
    {
        fprintf(stderr, "invalid string enlargment request size %d", needed);
        exit(0);
    }

    if (((Size)needed) >= (MaxAllocSize - (Size)str->len))
    {
        fprintf(stderr, "Cannot enlarge string buffer containing %d bytes by %d more btypes.",
                str->len, needed);
        exit(0);
    }
    needed += str->len + 1;

    if (needed <= str->maxlen)
        return;

    newlen = 2 * str->maxlen;
    while (needed > newlen)
        newlen = 2 * newlen;

    if (newlen > (int)MaxAllocSize)
        newlen = (int)MaxAllocSize;

    str->data = (char *)repalloc(str->data, newlen);

    str->maxlen = newlen;
}