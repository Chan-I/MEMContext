#include "mcxt.h"

int main()
{
    MemoryContext TopMemoryContext, CacheMemoryContext, ErrorMemoryContext;
    TopMemoryContext = (MemoryContext)MemoryContextCreate("TopMemoryContext", NULL);
    CacheMemoryContext = (MemoryContext)MemoryContextCreate("CacheMemoryContext", TopMemoryContext);
    ErrorMemoryContext = (MemoryContext)MemoryContextCreate("ErrorMemoryContext", TopMemoryContext);
    printf("name:: %s\n", TopMemoryContext->name);
    printf("name:: %s\n", TopMemoryContext->first_child->name);
    printf("name:: %s\n", TopMemoryContext->first_child->next_child->name);

    return 0;
}
