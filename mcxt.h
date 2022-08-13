#ifndef __MCXT_H__
#define __MCXT_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

typedef size_t Size;
typedef unsigned long int uintptr_t;
typedef void *AllocPointer;

typedef void (*MemoryContextCallbackFunction)(void *arg);

typedef struct MemoryContextCallback
{
    MemoryContextCallbackFunction func; /* function to call */
    void *arg;                          /* argument to pass it */
    struct MemoryContextCallback *next; /* next in list of callbacks */
} MemoryContextCallback;

typedef struct MemoryContextData *MemoryContext;
typedef struct MemoryContextData
{
    MemoryContext parent;             /* NULL if toplevel context */
    MemoryContext first_child;        /* head of linked list of children */
    MemoryContext next_child;         /* next child of same parent */
    MemoryContext prev_child;         /* prev child of same parent */
    char *name;                       /* context name */
    char isReset;                     /* T = nospace alloced since last reset */
    MemoryContextCallback *reset_cbs; /* list of reset/delete callbacks */
} MemoryContextData;

extern MemoryContext TopMemoryContext;
extern MemoryContext ErrorMemoryContext;
extern MemoryContext CurrentMemoryContext;

extern MemoryContext AllocSetContextCreateInternal(MemoryContext parent,
                                                   const char *name,
                                                   Size minContextSize,
                                                   Size initBlockSize,
                                                   Size maxBlockSize);

extern MemoryContext MemoryContextSwitchTo(MemoryContext context);
extern void MemoryContextDelete(MemoryContext context);
extern void MemoryContextInit(void);

#define Max(_x, _y) ((_x) > (_y) ? (_x) : (_y))
#define TYPEALIGN(ALIGNVAL, LEN) \
    (((uintptr_t)(LEN) + ((ALIGNVAL)-1)) & ~((uintptr_t)((ALIGNVAL)-1)))
#define MAXALIGN(LEN) TYPEALIGN(8, (LEN))

#define ALLOC_BLOCKHDRSZ MAXALIGN(sizeof(AllocBlockData))
#define ALLOC_CHUNKHDRSZ sizeof(struct AllocChunkData)

#define AllocPointerGetChunk(ptr) \
    ((AllocChunk)(((char *)(ptr)) - ALLOC_CHUNKHDRSZ))

#define AllocChunkGetPointer(chk) \
    ((AllocPointer)((char *)(chk)) + ALLOC_CHUNKHDRSZ)

#define AllocSetContextCreate AllocSetContextCreateInternal

#define ALLOC_MINBITS 3
#define ALLOC_CHUNK_FRACTION 4
#define ALLOCSET_NUM_FREELISTS 11
#define MEMSET_LOOP_LIMIT 1024
#define MAX_FREE_CONTEXTS 100
#define ALLOC_CHUNK_LIMIT (1 << (ALLOCSET_NUM_FREELISTS - 1 + ALLOC_MINBITS))

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

static const unsigned char LogTable256[256] =
    {
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

typedef struct StringInfoData
{
    char *data;
    int len;
    int maxlen;
    int cursor;
} StringInfoData;
typedef StringInfoData *StringInfo;

extern StringInfo makeStringInfo(void);
extern void initStringInfo(StringInfo str);
extern void resetStringInfo(StringInfo str);
extern void appendStringInfo(StringInfo str, const char *fmt, ...);

#define ALLOCSET_DEFAULT_MINSIZE 0
#define ALLOCSET_DEFAULT_INITSIZE (8 * 1024)
#define ALLOCSET_DEFAULT_MAXSIZE (8 * 1024 * 1024)
#define ALLOCSET_DEFAULT_SIZES \
    ALLOCSET_DEFAULT_MINSIZE, ALLOCSET_DEFAULT_INITSIZE, ALLOCSET_DEFAULT_MAXSIZE

#define ALLOCSET_SMALL_MINSIZE 0
#define ALLOCSET_SMALL_INITSIZE (1 * 1024)
#define ALLOCSET_SMALL_MAXSIZE (8 * 1024)
#define ALLOCSET_SMALL_SIZES \
    ALLOCSET_SMALL_MINSIZE, ALLOCSET_SMALL_INITSIZE, ALLOCSET_SMALL_MAXSIZE

extern void *palloc(Size size);
extern void pfree(void *pointer);
extern void *repalloc(void *pointer, Size size);

#endif
