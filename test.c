#include "mcxt.h"

MemoryContext CurrentMemoryContext = NULL;
MemoryContext TopMemoryContext = NULL;
MemoryContext ErrorMemoryContext = NULL;

static inline MemoryContext
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
                                             0,
                                             8 * 1024,
                                             8 * 1024 * 1024);
    CurrentMemoryContext = TopMemoryContext;

    ErrorMemoryContext = AllocSetContextCreate(TopMemoryContext,
                                               "ErrorMemoryContext",
                                               8 * 1024,
                                               8 * 1024,
                                               8 * 1024);
}

int main()
{
    MemoryContext oldcontext;
    MemoryContextInit();
    printf("name:: %s\n", TopMemoryContext->name);
    printf("name:: %s\n", TopMemoryContext->first_child->name);

    oldcontext = MemoryContextSwitchTo(ErrorMemoryContext);
    printf("\n\nCurrentContext:\nname:: %s\n", CurrentMemoryContext->name);

    return 0;
}
