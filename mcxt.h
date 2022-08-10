#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef size_t Size;
typedef unsigned long int uintptr_t;
typedef void *AllocPointer;

#define ALLOC_MINBITS 3
#define ALLOC_CHUNK_FRACTION 4
#define ALLOCSET_NUM_FREELISTS 11
#define MEMSET_LOOP_LIMIT 1024
#define ALLOC_CHUNK_LIMIT (1 << (ALLOCSET_NUM_FREELISTS - 1 + ALLOC_MINBITS))

#define Max(_x, _y) ((_x) > (_y) ? (_x) : (_y))

#define ALLOCSET_DEFAULT_MINSIZE 0
#define ALLOCSET_DEFAULT_INITSIZE (8 * 1024)
#define ALLOCSET_DEFAULT_MAXSIZE (8 * 1024 * 1024)

#define ALLOCSET_SMALL_MINSIZE 0
#define ALLOCSET_SMALL_INITSIZE (8 * 1024)
#define ALLOCSET_SMALL_MAXSIZE (8 * 1024 * 1024)

#define TYPEALIGN(ALIGNVAL, LEN) \
    (((uintptr_t)(LEN) + ((ALIGNVAL)-1)) & ~((uintptr_t)((ALIGNVAL)-1)))

#define SHORTALIGN(LEN) TYPEALIGN(2, (LEN))
#define INTALIGN(LEN) TYPEALIGN(4, (LEN))
#define LONGALIGN(LEN) TYPEALIGN(8, (LEN))
#define DOUBLEALIGN(LEN) TYPEALIGN(8, (LEN))
#define MAXALIGN(LEN) TYPEALIGN(8, (LEN))
/* MAXALIGN covers only built-in types, not buffers */
#define BUFFERALIGN(LEN) TYPEALIGN(32, (LEN))
#define CACHELINEALIGN(LEN) TYPEALIGN(128, (LEN))

#define ALLOC_BLOCKHDRSZ MAXALIGN(sizeof(AllocBlockData))
#define ALLOC_CHUNKHDRSZ sizeof(struct AllocChunkData)

#define AllocPointerGetChunk(ptr) \
    ((AllocChunk)((char *)(ptr)) - ALLOC_CHUNKHDRSZ)

#define AllocChunkGetPointer(chk) \
    ((AllocPointer)((char *)(chk)) + ALLOC_CHUNKHDRSZ)

#define AllocSetContextCreate AllocSetContextCreateInternal

#define MemSetAligned(start, val, len)                     \
    do                                                     \
    {                                                      \
        long *_start = (long *)(start);                    \
        int _val = (val);                                  \
        Size _len = (len);                                 \
                                                           \
        if ((_len & (sizeof(long) - 1)) == 0 &&            \
            _val == 0 &&                                   \
            _len <= MEMSET_LOOP_LIMIT &&                   \
            MEMSET_LOOP_LIMIT != 0)                        \
        {                                                  \
            long *_stop = (long *)((char *)_start + _len); \
            while (_start < _stop)                         \
                *_start++ = 0;                             \
        }                                                  \
        else                                               \
            memset(_start, _val, _len);                    \
    } while (0)

static const unsigned char LogTable256[256] = {
    0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};

typedef struct MemoryContextData *MemoryContext;
typedef struct MemoryContextData
{
    MemoryContext parent;      /* NULL if toplevel context */
    MemoryContext first_child; /* head of linked list of children */
    MemoryContext next_child;  /* next child of same parent */
    MemoryContext prev_child;  /* prev child of same parent */
    char *name;                /* context name */
    char isReset;              /* T = nospace alloced since last reset */
} MemoryContextData;

typedef struct AllocBlockData *AllocBlock;
typedef struct AllocChunkData *AllocChunk;
typedef struct AllocSetContext
{
    MemoryContextData header;                    /* 内存上下文的结构体，主要用于将不同的内存结构形成上下文 */
    AllocBlock blocks;                           /* block 组成的链表，这里指向头部 */
    AllocChunk freelist[ALLOCSET_NUM_FREELISTS]; /* free chunk list */
    Size initBlockSize;                          /* init size of block */
    Size maxBlockSize;                           /* max size of block */
    Size nextBlockSize;                          /* next size of block */
    Size allocChunkLimit;                        /*  */
    AllocBlock keeper;                           /* keep this block over resets */
    int freeListIndex;                           /* index in freelist */
} AllocSetContext;

typedef AllocSetContext *AllocSet;

typedef struct AllocBlockData
{
    AllocSet aset;   /* AllocSet that owns this block */
    AllocBlock prev; /* prev block in this list */
    AllocBlock next; /* next block in this list */
    char *freeptr;   /* first address of free space in this block */
    char *endptr;    /* end address of free space in this block */
} AllocBlockData;

typedef struct AllocChunkData
{
    Size size; /* size */

    void *aset; /* 空间复用指针，如果为空则指向下一个也为空的chunk
                 * 如果不为空，则指向其所属的AllocSet
                 */
} AllocChunkData;

typedef struct AllocSetFreeList
{
    int num_free;                /* current list length */
    AllocSetContext *first_free; /* head of list */
} AllocSetFreeList;

void MemoryContextCreate(MemoryContext node, MemoryContext parent, const char *name);
static inline MemoryContext MemoryContextSwitchTo(MemoryContext context);
void MemoryContextInit(void);
MemoryContext AllocSetContextCreateInternal(MemoryContext parent, const char *name, Size minContextSize, Size initBlockSize, Size maxBlockSize);
static void *AllocSetAlloc(MemoryContext context, Size size);
static inline int AllocSetFreeIndex(Size size);