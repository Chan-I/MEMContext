#include "mcxt.h"

MemoryContext CurrentMemoryContext = NULL;

static inline MemoryContext
MemoryContextSwitchTo(MemoryContext context)
{
    MemoryContext old = CurrentMemoryContext;

    CurrentMemoryContext = context;
    return old;
}

/*
 *	 name:	MemoryContextCreate
 * 	 function:	初始化内存上下文结构
 *	 params：	parent	父节点
 *				name	内存上下文名称
 */
MemoryContext
MemoryContextCreate(const char *name, MemoryContext parent)
{
    MemoryContext node;
    int size = sizeof(MemoryContextData);
    int needSize = size + strlen(name) + 1;

    node = (MemoryContext)malloc(needSize);
    if (!node)
    {
        printf("error\n");
        exit(0);
    }

    memset(node, 0, size);
    node->parent = NULL;
    node->first_child = NULL;
    node->next_child = NULL;
    node->prev_child = NULL;
    node->isReset = 1;
    node->name = (char *)node + size;
    memcpy(node->name, name, strlen(name));

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
    return node;
}
