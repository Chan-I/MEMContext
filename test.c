#include "mcxt.h"
void MemTest(void);
void MemTest(void)
{
       MemoryContext oldcontext;
       MemoryContext MessageContext;
       MemoryContextInit();
       printf("Memory Context Content After Init()\n");
       printf("name:: %s\n", TopMemoryContext->name);
       printf("-->> name:: %s\n\n", ErrorMemoryContext->name);

       printf("Switch Current Memory Context To ErrorMemoryContext\n");
       oldcontext = MemoryContextSwitchTo(ErrorMemoryContext);
       printf("CurrentContext: %s\n\n", CurrentMemoryContext->name);

       printf("Add Memory Context Name: MessageContext\n");
       MessageContext = AllocSetContextCreate(oldcontext,
                                              "MessageContext",
                                              ALLOCSET_DEFAULT_SIZES);

       printf("name:: %s\n", TopMemoryContext->name);
       printf("-->> name::  %s\n", TopMemoryContext->first_child->name);
       printf("-->> -->> name:: %s\n\n", TopMemoryContext->first_child->next_child->name);

       printf("Switch Current Memory Context To MessageContext\n");
       oldcontext = MemoryContextSwitchTo(MessageContext);
       printf("CurrentContext: %s\n\n", CurrentMemoryContext->name);

       char *array = palloc(1024);
       char *array1 = palloc(3 * 1024);

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
                     "k5j1;lkj4;l1k23javs90d8ufsldkjfasdfasdfs",
              1024);

       memcpy(array1, "lkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lkj"
                      ";lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;"
                      "lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alk"
                      "sjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldkj"
                      "flkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lk"
                      "j;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj"
                      ";lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;al"
                      "ksjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldk"
                      "jflkalskdj;lkj;lakjlskjf;o23l4j;lkj;lsssfkj;l"
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

       printf("%s\n", array);
       printf("%s\n\n", array1);

       array1 = repalloc(array1, 4 * 1024);

       memcpy(array1, "lkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lkj"
                      ";lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj;"
                      "lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;alk"
                      "sjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldkj"
                      "flkalskdj;lkj;lakjlskjf;o23l4j;lkj;lfkj;lk"
                      "j;lkj(*YUokjasd;lkjf;2o3iu4;lkja;lkjf;alkj"
                      ";lkdjf;l23i4j1;l23kj;lksjdf;lkj(*&POIjf;al"
                      "ksjdf;lkj3;4lk5j1;lkj4;l1k23javs90d8ufsldk"
                      "jflkalskdj;lkj;lakjlskjf;o23l4j;lkj;lsssfkj;l"
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
              4 * 1024);

       printf("%s\n\n", array1);

       pfree(array);
       pfree(array1);

       fprintf(stdout, "\nStringInfoData Test ....\n\n");
       fflush(stdout);

       StringInfo str = makeStringInfo();
       appendStringInfo(str, "%d\t%f\t%s", 1, 12.001, "alskdjlaskjlksjdf");

       printf("StringInfo 1: \n\t\t%s\n", str->data);

       appendStringInfo(str, "\t....This is appendStringInfo test....\n");

       printf("StringInfo 2: \n\t\t%s\n", str->data);

       resetStringInfo(str);

       printf("reset StringInfo: %s\n", str->data);

       printf("\n\nCurrentContext: %s\n\n", CurrentMemoryContext->name);
       MemoryContextDelete(TopMemoryContext);
}

int main()
{
       MemTest();
}