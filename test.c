#include "mcxt.h"

int main()
{
       MemoryContext oldcontext;
       MemoryContext MessageContext;
       MemoryContextInit();
       printf("name:: %s\n", TopMemoryContext->name);
       printf("\tname:: %s\n", TopMemoryContext->first_child->name);

       oldcontext = MemoryContextSwitchTo(ErrorMemoryContext);
       printf("\n\nCurrentContext: %s\n\n", CurrentMemoryContext->name);

       MessageContext = AllocSetContextCreate(oldcontext,
                                              "MessageContext",
                                              512,
                                              1024,
                                              2 * 1024);

       printf("name:: %s\n", TopMemoryContext->name);
       printf("\tname:: %s\n", TopMemoryContext->first_child->name);
       printf("\t\tname:: %s\n", TopMemoryContext->first_child->next_child->name);

       oldcontext = MemoryContextSwitchTo(MessageContext);
       printf("\n\nCurrentContext: %s\n\n", CurrentMemoryContext->name);

       char *array = palloc(1024);
       memcpy(array, "lkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lkj"
                     ";lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;"
                     "lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alk"
                     "sjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldkj"
                     "flkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lk"
                     "j;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj"
                     ";lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;al"
                     "ksjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldk"
                     "jflkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;l"
                     "kj;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alk"
                     "j;lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;a"
                     "lksjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldk"
                     "jflkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lkj;lkj"
                     "(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;lkdjf;l23i4j1"
                     ";l23kj;lksjdf;lkj(*&POIjf;alksjdf;lkj3;4lk5j1;lkj4;l1"
                     "k23javs90d8ufsldkjflkalskdj;lkj;lakjlskjf;o23l4j;lkj;l"
                     "fkj;lkj;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;lkdjf"
                     ";l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alksjdf;lkj3;4lk5j1;l"
                     "kj4;l1k23javs90d8ufsldkjflkalskdj;lkj;lakjlskjf;o23l4j"
                     ";lkj;lfkj;lkj;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj"
                     ";lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alksjdf;lkj3;4l"
                     "k5j1;lkj4;l1k23javs90d8ufsldkjfasdfasdf",
              1024);

       printf("%s\n", array);

       char *array1 = palloc(3 * 1024);

       memcpy(array1, "lkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lkj"
                      ";lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;"
                      "lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alk"
                      "sjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldkj"
                      "flkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lk"
                      "j;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj"
                      ";lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;al"
                      "ksjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldk"
                      "jflkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;l"
                      "kj;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alk"
                      "j;lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;a"
                      "lksjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldk"
                      "jflkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lkj;lkj"
                      "(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;lkdjf;l23i4j1"
                      ";l23kj;lksjdf;lkj(*&POIjf;alksjdf;lkj3;4lk5j1;lkj4;l1"
                      "k23javs90d8ufsldkjflkalskdj;lkj;lakjlskjf;o23l4j;lkj;l"
                      "fkj;lkj;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;lkdjf"
                      ";l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alksjdf;lkj3;4lk5j1;l"
                      "kj4;l1k23javs90d8ufsldkjflkalskdj;lkj;lakjlskjf;o23l4j"
                      ";lkj;lfkj;lkj;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj"
                      ";lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alksjdf;lkj3;4l"
                      "k5j1;lkj4;l1k23javs90d8ufsldkjfasdfasdf"
                      "lkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lkj"
                      ";lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;"
                      "lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alk"
                      "sjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldkj"
                      "flkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lk"
                      "j;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj"
                      ";lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;al"
                      "ksjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldk"
                      "jflkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;l"
                      "kj;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alk"
                      "j;lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;a"
                      "lksjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldk"
                      "jflkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lkj;lkj"
                      "(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;lkdjf;l23i4j1"
                      ";l23kj;lksjdf;lkj(*&POIjf;alksjdf;lkj3;4lk5j1;lkj4;l1"
                      "k23javs90d8ufsldkjflkalskdj;lkj;lakjlskjf;o23l4j;lkj;l"
                      "fkj;lkj;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;lkdjf"
                      ";l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alksjdf;lkj3;4lk5j1;l"
                      "kj4;l1k23javs90d8ufsldkjflkalskdj;lkj;lakjlskjf;o23l4j"
                      ";lkj;lfkj;lkj;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj"
                      ";lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alksjdf;lkj3;4l"
                      "k5j1;lkj4;l1k23javs90d8ufsldkjfasdfasdf"
                      "lkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lkj"
                      ";lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;"
                      "lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alk"
                      "sjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldkj"
                      "flkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lk"
                      "j;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj"
                      ";lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;al"
                      "ksjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldk"
                      "jflkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;l"
                      "kj;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alk"
                      "j;lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;a"
                      "lksjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldk"
                      "jflkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lkj;lkj"
                      "(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;lkdjf;l23i4j1"
                      ";l23kj;lksjdf;lkj(*&POIjf;alksjdf;lkj3;4lk5j1;lkj4;l1"
                      "k23javs90d8ufsldkjflkalskdj;lkj;lakjlskjf;o23l4j;lkj;l"
                      "fkj;lkj;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;lkdjf"
                      ";l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alksjdf;lkj3;4lk5j1;l"
                      "kj4;l1k23javs90d8ufsldkjflkalskdj;lkj;lakjlskjf;o23l4j"
                      ";lkj;lfkj;lkj;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj"
                      ";lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alksjdf;lkj3;4l"
                      "k5j1;lkj4;l1k23javs90d8ufsldkjfasdfasdf",
              3 * 1024);
       printf("%s\n", array1);

       pfree(array);
       pfree(array1);

       printf("\n\nCurrentContext: %s\n\n", CurrentMemoryContext->name);
       MemoryContextDelete(TopMemoryContext);

       return 0;
}
