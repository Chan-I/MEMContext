#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef size_t Size;
#define ALLOCSET_NUM_FREELISTS 11

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
    Size size;

    void *aset;
} AllocChunkData;

extern MemoryContext MemoryContextCreate(const char *name, MemoryContext parent);
static inline MemoryContext MemoryContextSwitchTo(MemoryContext context);