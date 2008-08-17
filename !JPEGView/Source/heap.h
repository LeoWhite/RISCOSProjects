#ifndef __heap_h
#define __heap_h

#include "kernel.h"

#define HEAP_VERIFY_NF(heap, b) Heap_verify(heap, b, __FILE__, __LINE__, FALSE, FALSE)
#define HEAP_REPORT(heap, b)    Heap_verify(heap, b, __FILE__, __LINE__, TRUE, TRUE)

/* These operate on a given heap */
extern int   Heap_verify(void *heap, void *block, const char *file, unsigned long line, int sayok, int fix);
extern void *Heap_malloc(void *heap, size_t size, const char *file, unsigned long line);
extern void *Heap_calloc(void *heap, size_t count, size_t size, const char *file, unsigned long line);
extern void *Heap_realloc(void *heap, void *oldptr, size_t size, const char *file, unsigned long line);
extern void  Heap_free(void *heap, void *block, const char *file, unsigned long line);
extern char *Heap_strdup(void *heap, const char *s, const char *file, unsigned long line);
extern char *Heap_strdupcat(void *heap, const char *s1, const char *s2, const char *file, unsigned long line);

#define verify_in(h,b)        Heap_verify(h, b, __FILE__, __LINE__, FALSE, TRUE)
#define malloc_in(h,s)        Heap_malloc(h, s, __FILE__, __LINE__)
#define realloc_in(h,b,s)     Heap_realloc(h, b, s, __FILE__, __LINE__)
#define calloc_in(h,n,s)      Heap_calloc(h, n, s, __FILE__, __LINE__)
#define free_in(h,b)          Heap_free(h, b, __FILE__, __LINE__)
#define strdup_in(h,b)        Heap_strdup(h, b, __FILE__, __LINE__)
#define strdupcat_in(h,s1,s2) Heap_strdupcat(h, s1,s2, __FILE__, __LINE__)


/* These all operate on the Misc heap */
/*extern void *heap_misc;
#define verify(b)        verify_in(heap_misc, b)
#define malloc(s)        malloc_in(heap_misc, s)
#define calloc(n,s)      calloc_in(heap_misc, n, s)
#define realloc(b,s)     realloc_in(heap_misc, b, s)
#define free(b)          free_in(heap_misc, b)
#define strdup(b)        strdup_in(heap_misc, b)
#define strdupcat(s1,s2) strdupcat_in(heap_misc, s1,s2)*/

/* These all operate on the Libs heap */
extern void *Lib_malloc(size_t size);
extern void *Lib_calloc(size_t count, size_t size);
extern void *Lib_realloc(void *oldptr, size_t size);
extern void  Lib_free(void *block);
extern char *Lib_strdup(const char *s);


extern void Heap_Initialise(void);
extern void Heap_InitialisationDone(void);
extern void *Heap_Create(const char *name, int level, int maxsize, void (*squeeze)(void));
extern void Heap_SetLevel(void *heap, int level);
extern int  Heap_Shrink(void *vzs);
extern void Heap_ShrinkInit(void);
extern void Heap_ShrinkFinalise(void);

#ifdef DEBUG
extern void Heap_MemDump(char *message, void *addr, int size);
extern void Heap_List(void *heap);
extern void Heap_Dump(void *heap);
extern void Heap_Stats(void *heap);
extern void Heap_VerifyBlocks(void *heap);
extern void Heap_ListAll(void);
extern void Heap_DumpAll(void);
extern void Heap_StatsAll(void);
extern void Heap_VerifyBlocksAll(void);
#endif

#endif
