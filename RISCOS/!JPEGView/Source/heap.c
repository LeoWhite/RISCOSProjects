/* C library (apart from the ones in common.h) */
#include "swis.h"

/* Browser */
#include "common.h"
#include "heap.h"

/*#define HEAP_DEBUG*/

#define STOP_IN_YER_TRACKS

#define MAlloc_Initialise      0x4EDC0
#define MAlloc_Finalise        0x4EDC1
#define MAlloc_Claim           0x4EDC2
#define MAlloc_Free            0x4EDC3
#define MAlloc_Vectors         0x4EDC4
#define MAlloc_Resize          0x4EDC5
#define MAlloc_GetBlockSize    0x4EDC6
#define MAlloc_EnumerateBlocks 0x4EDC7
#define MAlloc_Shrink          0x4EDC8
#define MAlloc_BlockCopy       0x4EDC9
#define MAlloc_ShiftBlock      0x4EDCA

#define HEAP_HEADBUF  0xDEDEDE01
#define HEAP_HEADCHK  0xDEDEDE02
#define HEAP_TAILBUF  0xAFAFAF01
#define HEAP_TAILCHK  0xAFAFAF02
#define HEAP_BUFSIZE  8

void *heap_libs; /* libraries */
void *heap_misc; /* everything else */

typedef struct heap_data
{
  struct heap_data *next;
  void             *heap;
  int               level;
  void            (*squeeze)(void);
  unsigned int      no_dump : 1;
} heap_data;


static heap_data *heaplist;

/* we use this to not list blocks that were claimed on startup when a block
 * list is requested.
 */
static int heap_initialising = 1;

#ifdef DEBUG

/* Used in debug versions ------------------------------------------------------------------ */

static int first_failure = 1; /* use to do a heapstat only on the
                                 first failed malloc/realloc/calloc */

typedef struct
{
  int            prebuf[HEAP_BUFSIZE];
  int            check1;
  void          *heap;
  const char    *claim_file;
  unsigned long  claim_line;
  const char    *free_file;
  unsigned long  free_line;
  int            alive:1;
  int            initialisation:1;
  int            user_size;
  int            total_size;
  int            check2;
  int            postbuf[HEAP_BUFSIZE];
} heap_info;

#define IN_S sizeof(heap_info)

/* Structure of the debug heap blocks:

   Offset		Contents

   0			A heap_info structure
   IN_S			The user block (user_size in length)
   IN_S+user_size	Pad to word alignment
   total_size-IN_S	A heap_info structure (same info as above)

   Things reported from verify refer to the following:
   "Header prebuf"	Space before the header information (the start of the real heap block)
   "Header postbuf"	Space between the header info and the user block (the user block is what malloc returns)
   "Tail prebuf"	Space between the end of the user block and the words before the tail information
   "Tail postbuf"	Space after the tail information up to the end of the real heap block

*/


static void heap_wordfill(int *addr, int size, int word)
{
  while (size-=4, size >= 0)
    *addr++ = word;
}

static int heap_counterrors(int *addr, int count, int word)
{
  int err = 0;

  while (count--)
    if (*addr++ != word)
      err++;

  return err;
}

static void heap_putinfo(void *heap, void *wblk, int user_size, int total_size, bool clear, const char *file, unsigned long line)
{
  heap_info *infoH = (heap_info*) wblk;
  heap_info *infoT = (heap_info*) ((char*)wblk + total_size - IN_S);

  if (clear)
    memset(wblk, '\0', total_size);

  heap_wordfill((int*)infoH, IN_S, HEAP_HEADBUF);
  heap_wordfill((int*)infoT, IN_S, HEAP_TAILBUF);

  infoH->check1     = HEAP_HEADCHK;
  infoH->heap       = heap;
  infoH->claim_file = file;
  infoH->claim_line = line;
  infoH->free_file  = NULL;
  infoH->free_line  = 0;
  infoH->alive      = TRUE;
  infoH->initialisation = heap_initialising;
  infoH->user_size  = user_size;
  infoH->total_size = total_size;
  infoH->check2     = HEAP_HEADCHK;

  infoT->check1     = HEAP_TAILCHK;
  infoT->heap       = heap;
  infoT->claim_file = file;
  infoT->claim_line = line;
  infoT->free_file  = NULL;
  infoT->free_line  = 0;
  infoT->alive      = TRUE;
  infoT->initialisation = heap_initialising;
  infoT->user_size  = user_size;
  infoT->total_size = total_size;
  infoT->check2     = HEAP_TAILCHK;
}

static char *heap_name(void *heap)
{
  int num = ((int*)heap)[2];
  char *name;

  if (heap == heap_libs)
    return "JPEGView (Libs)";

  _swi(OS_DynamicArea, _INR(0,1)|_OUT(8), 2, num, &name);
  return name;
}


/* Verifies the validity of a block. Returns:
   -1  if block is completely broken
    0  if block is ok
    1  if block was broken, but has been fixed
 */

int Heap_verify(void *heap, void *block, const char *file, unsigned long line, int sayok, int fix)
{
  void *wblk = (char*)block - IN_S;
  _kernel_oserror *e;
  int invalid;
  char *fixed;
  char msg[128];

  if (file)
    sprintf(msg, "Verify block %p from %s line %ld:\n", block, file, line);
  else
    sprintf(msg, "Verify block %p:\n", block);

  fixed = fix?"(fixed)":"(not fixed)";

  _swix(OS_ValidateAddress, _INR(0, 1) | _OUT(_FLAGS), wblk, block, &invalid);
  if (!(invalid & _C))
  {
    int total_size;
    int errors, broken = FALSE;
    heap_info *infoH = (heap_info*) wblk;
    heap_info *infoT;
    int Herr,Terr;

    if ((e = _swix(MAlloc_GetBlockSize, _INR(1, 2) | _OUT(3), heap, wblk, &total_size)))
    {
      total_size = infoH->total_size;
      fprintf(stderr, "%s %s, assuming total size is %i\n", msg, e->errmess, total_size);
    }

    infoT = (heap_info*) ((char*)wblk + total_size - IN_S);

    if ((errors = heap_counterrors(infoH->prebuf, HEAP_BUFSIZE, HEAP_HEADBUF)))
    {
      broken = TRUE;
      fprintf(stderr, "%s Header - %i out of %i words of prebuf have been corrupted %s\n", msg, errors, HEAP_BUFSIZE, fixed);
      Heap_MemDump("Header prebuf", infoH->prebuf, sizeof(infoH->prebuf));
      if (fix && infoH->alive) heap_wordfill(infoH->prebuf, HEAP_BUFSIZE, HEAP_HEADBUF);
      /* NB. malloc module uses first two words of free block for a linked list - 'fixing' them
       * is bad! */
    }

    if ((errors = heap_counterrors(infoH->postbuf, HEAP_BUFSIZE, HEAP_HEADBUF)))
    {
      broken = TRUE;
      fprintf(stderr, "%s Header - %i out of %i words of postbuf have been corrupted %s\n", msg, errors, HEAP_BUFSIZE, fixed);
      Heap_MemDump("Header postbuf", infoH->postbuf, sizeof(infoH->postbuf));
      if (fix) heap_wordfill(infoH->postbuf, HEAP_BUFSIZE, HEAP_HEADBUF);
    }

    if ((errors = heap_counterrors(infoT->prebuf, HEAP_BUFSIZE, HEAP_TAILBUF)))
    {
      broken = TRUE;
      fprintf(stderr, "%s Tail - %i out of %i words of prebuf have been corrupted %s\n", msg, errors, HEAP_BUFSIZE, fixed);
      Heap_MemDump("Tail prebuf", infoT->prebuf, sizeof(infoT->prebuf));
      if (fix) heap_wordfill(infoT->prebuf, HEAP_BUFSIZE, HEAP_TAILBUF);
    }

    if ((errors = heap_counterrors(infoT->postbuf, HEAP_BUFSIZE, HEAP_TAILBUF)))
    {
      broken = TRUE;
      fprintf(stderr, "%s Tail - %i out of %i words of postbuf have been corrupted %s\n", msg, errors, HEAP_BUFSIZE, fixed);
      Heap_MemDump("Tail postbuf", infoT->postbuf, sizeof(infoT->postbuf));
      if (fix) heap_wordfill(infoT->postbuf, HEAP_BUFSIZE, HEAP_TAILBUF);
    }

    Herr = (infoH->check1 != HEAP_HEADCHK || infoH->check2 != HEAP_HEADCHK);
    Terr = (infoT->check1 != HEAP_TAILCHK || infoT->check2 != HEAP_TAILCHK);

    if (Herr || Terr)
      broken = TRUE;

    if (Herr && Terr)
    {
      fprintf(stderr, "%s Both info blocks have been splatted %s\n", msg, fixed);
      Heap_MemDump("Head info", infoH, sizeof(heap_info));
      Heap_MemDump("Tail info", infoT, sizeof(heap_info));
      if (fix)
      {
        infoH->check1     = HEAP_HEADCHK;
        infoH->heap       = heap;
        infoH->claim_file = "Has been splatted";
        infoH->claim_line = 0;
        infoH->free_file  = "Has been splatted";
        infoH->free_line  = 0;
        infoH->alive      = (((int*)block)[0] != 0xafafafaf);
        infoH->user_size  = total_size - 2*IN_S;
        infoH->total_size = total_size;
        infoH->check2     = HEAP_HEADCHK;

        infoT->check1     = HEAP_TAILCHK;
        infoT->heap       = infoH->heap;
        infoT->claim_file = infoH->claim_file;
        infoT->claim_line = infoH->claim_line;
        infoT->free_file  = infoH->claim_file;
        infoT->free_line  = infoH->claim_line;
        infoT->alive      = infoH->alive;
        infoT->user_size  = infoH->user_size;
        infoT->total_size = infoH->total_size;
        infoT->check2     = HEAP_TAILCHK;
      }
    }
    else if (Herr)
    {
      fprintf(stderr, "%s Head info block has been splatted %s\n", msg, fixed);
      Heap_MemDump("Head info", infoH, sizeof(heap_info));
      if (fix)
      {
        infoH->check1     = HEAP_HEADCHK;
        infoH->heap       = infoT->heap;
        infoH->claim_file = infoT->claim_file;
        infoH->claim_line = infoT->claim_line;
        infoH->free_file  = infoT->free_file;
        infoH->free_line  = infoT->free_line;
        infoH->alive      = infoT->alive;
        infoH->user_size  = infoT->user_size;
        infoH->total_size = infoT->total_size;
        infoH->check2     = HEAP_HEADCHK;
      }
    }
    else if (Terr)
    {
      fprintf(stderr, "%s Tail info block has been splatted %s\n", msg, fixed);
      Heap_MemDump("Tail info", infoT, sizeof(heap_info));
      if (fix)
      {
        infoT->check1     = HEAP_TAILCHK;
        infoT->heap       = infoH->heap;
        infoT->claim_file = infoH->claim_file;
        infoT->claim_line = infoH->claim_line;
        infoT->free_file  = infoH->free_file;
        infoT->free_line  = infoH->free_line;
        infoT->alive      = infoH->alive;
        infoT->user_size  = infoH->user_size;
        infoT->total_size = infoH->total_size;
        infoT->check2     = HEAP_TAILCHK;
      }
    }

    if (!infoH->alive)
      fprintf(stderr, "%s Block has been freed from %s line %ld\n", msg, infoH->free_file, infoH->free_line);

    if (infoH->heap != heap)
    {
      fprintf(stderr, "%s Block belongs in %s\n", msg, heap_name(infoH->heap));
      broken = TRUE;
    }

    if ((sayok && infoH->alive) || broken)
    {
      if (broken)
      {
        fprintf(stderr, "Block claimed by %s line %ld (%i bytes) has %sbeen fixed\n",
                infoH->claim_file, infoH->claim_line, infoH->user_size, fix?"":"not ");
      }
      else
      {
        fprintf(stderr, "%s Block claimed by %s line %ld (%i bytes) is still alive\n",
                msg, infoH->claim_file, infoH->claim_line, infoH->user_size);
        Heap_MemDump("block contents", block, infoH->user_size < 32 ? infoH->user_size : 32);
      }
    }

    return broken? 1 : 0;
  }
  else
  {
    fprintf(stderr, "%s Block is in invalid memory\n", msg);
    return -1;
  }
}

void Heap_free(void *heap, void *block, const char *file, unsigned long line)
{
  void *wblk = (char*)block - IN_S;
  _kernel_oserror *e;

  if (Heap_verify(heap, block, file, line, FALSE, TRUE)>=0)
  {
    heap_info *infoH = (heap_info*) wblk;
    heap_info *infoT = (heap_info*) ((char*)wblk + infoH->total_size - IN_S);

    if (strcmp(infoH->claim_file, file) != 0 && 0)
    {
      DBUG(fprintf(stderr, "Warning: block claimed by %s:%ld freed by %s:%ld\n",
               infoH->claim_file, infoH->claim_line,
               file, line));
    }

    memset(block, 0xAF, infoH->user_size);
    infoH->free_file = file;
    infoH->free_line = line;
    infoH->alive     = FALSE;
    infoT->free_file = file;
    infoT->free_line = line;
    infoT->alive     = FALSE;

    if(heap != infoH->heap) {
      // Oh dear, someone's attempting to free this block from a different heap
      DBUG(fprintf(stderr, "Warning: block allocated in Heap %s, but specified heap to free from is %s.  Fixing.\n",
             heap_name(infoH->heap), heap_name(heap)));

      if ((e = _swix(MAlloc_Free, _INR(1, 2), infoH->heap, wblk)))
        fprintf(stderr, "Something went horribly wrong when freeing %p: %s\n", block, e->errmess);
    }
    else {
      if (e = _swix(MAlloc_Free, _INR(1, 2), heap, wblk))
        fprintf(stderr, "Something went horribly wrong when freeing %p: %s\n", block, e->errmess);
    }
  }
}

void *Heap_calloc(void *heap, size_t count, size_t size, const char *file, unsigned long line)
{
  int user_size = count * size;
  int total_size = ((user_size+3)&~3)+2*IN_S;
  void *wblk;

  if (!_swix(MAlloc_Claim, _IN(1) | _IN(3) | _OUT(2), heap, total_size, &wblk))
  {
    if (!_swix(MAlloc_GetBlockSize, _INR(1, 2) | _OUT(3), heap, wblk, &total_size))
    {
      heap_putinfo(heap, wblk, user_size, total_size, TRUE, file, line);
      return (char*)wblk + IN_S;
    }
  }

  if (first_failure)
  {
    Heap_Stats(heap);
    first_failure = 0;
  }
  return NULL;
}

void *Heap_malloc(void *heap, size_t size, const char *file, unsigned long line)
{
  int total_size = ((size+3)&~3)+2*IN_S;
  void *wblk;

  if (!_swix(MAlloc_Claim, _IN(1) | _IN(3) | _OUT(2), heap, total_size, &wblk))
  {
    if (!_swix(MAlloc_GetBlockSize, _INR(1, 2) | _OUT(3), heap, wblk, &total_size))
    {
      heap_putinfo(heap, wblk, size, total_size, 0, file, line);
      return (char*)wblk + IN_S;
    }
  }

  if (first_failure)
  {
    Heap_Stats(heap);
    first_failure = 0;
  }
  return NULL;
}

void *Heap_realloc(void *heap, void *oldptr, size_t size, const char *file, unsigned long line)
{
  if (oldptr == NULL)
    return Heap_malloc(heap, size, file, line);
  else if (size == 0)
  {
    Heap_free(heap, oldptr, file, line);
    return NULL;
  }
  else
  {
    void *newptr;
    int new_total = ((size+3)&~3)+2*IN_S;
    _kernel_oserror *e;

    if (!(e=_swix(MAlloc_Resize, _INR(1, 3) | _OUT(2), heap, (char*)oldptr - IN_S, new_total, &newptr)))
    {
      if (!(e=_swix(MAlloc_GetBlockSize, _INR(1, 2) | _OUT(3), heap, newptr, &new_total)))
      {
        heap_putinfo(heap, newptr, size, new_total, FALSE, file, line);
        return (char*)newptr + IN_S;
      }
      else
        DBUG(fprintf(stderr, "GetBlockSize failed: %s\n", e->errmess));
    }
    else
      DBUG(fprintf(stderr, "Resize failed: %s\n", e->errmess));

    if (first_failure)
    {
      Heap_Stats(heap);
      first_failure = 0;
    }
    return NULL;
  }
}

#else

/* Used in release versions ------------------------------------------------------------------ */

int Heap_verify(void *heap, void *block, const char *file, unsigned long line, int sayok, int fix)
{
  return 0;
}

void Heap_free(void *heap, void *block, const char *file, unsigned long line)
{
  _swix(MAlloc_Free, _INR(1, 2), heap, block);
}

void *Heap_calloc(void *heap, size_t count, size_t size, const char *file, unsigned long line)
{
  void *block;
  int total = count * size;

  if (_swix(MAlloc_Claim, _IN(1) | _IN(3) | _OUT(2), heap, total, &block))
    return NULL;

  memset(block, 0, total);
  return block;
}

void *Heap_malloc(void *heap, size_t size, const char *file, unsigned long line)
{
  void *block;

  return _swix(MAlloc_Claim, _IN(1) | _IN(3) | _OUT(2), heap, size, &block)? NULL : block;
}

void *Heap_realloc(void *heap, void *oldptr, size_t size, const char *file, unsigned long line)
{
  if (oldptr == NULL)
    return Heap_malloc(heap, size, file, line);
  else if (size == 0)
  {
    Heap_free(heap, oldptr, file, line);
    return NULL;
  }
  else
  {
    void *newptr;

    return _swix(MAlloc_Resize, _INR(1, 3) | _OUT(2), heap, (char*)oldptr, size, &newptr)? NULL : newptr;
  }
}

#endif

/* Used in both ------------------------------------------------------------------------------- */

char *Heap_strdup(void *heap, const char *s, const char *file, unsigned long line)
{
  char *str;
  int   size;

  if (!s)
  {
    DBUG(fprintf(stderr, "strdup called with NULL from %s/%ld", file, line));
    return NULL;
  }

  if (str = Heap_malloc(heap, size = (strlen(s) + 1), file, line))
  {
    memcpy(str, s, size);
    return str;
  }
  return NULL;
}

char *Heap_strdupcat(void *heap, const char *s1, const char *s2, const char *file, unsigned long line)
{
  int  l1 = strlen(s1);
  int  l2 = strlen(s2);
  char *str;

  if (str = Heap_malloc(heap, l1+l2+1, file, line))
  {
    memcpy(str, s1, l1);
    memcpy(str+l1, s2, l2+1);
  }
  return str;
}



/* -------------------------------------------------------------------------------------------- */
/* These are replacements for the stubs memory alloc functions					*/
/* Use these if you are manipulating a block claimed by e.g. DeskLib				*/

void Lib_free(void *block)
{
  _swix(MAlloc_Free, _INR(1, 2), heap_libs, block);
}

void *Lib_calloc(size_t count, size_t size)
{
  size_t  total = count * size;
  void   *block;

  if (_swix(MAlloc_Claim, _IN(1) | _IN(3) | _OUT(2), heap_libs, total, &block))
    return NULL;
  memset(block, 0, total);
  return block;
}

void *Lib_malloc(size_t size)
{
  void *block;

  if (_swix(MAlloc_Claim, _IN(1) | _IN(3) | _OUT(2), heap_libs, size, &block))
    return NULL;

  /* We need to zero the block because at least one library relies on it.
   * editlib for example inserts extra CRs in the text.
   */
  memset(block, 0, size);
  return block;
}

void *Lib_realloc(void *oldptr, size_t size)
{
  if (!oldptr)
    return Lib_malloc(size);
  else if (!size)
  {
    Lib_free(oldptr);
    return NULL;
  }
  else
  {
    char *newptr;

    if (_swix(MAlloc_Resize, _INR(1, 3) | _OUT(2), heap_libs, oldptr, size, &newptr))
      return NULL;
    return newptr;
  }
}

char *Lib_strdup(const char *s)
{
  char *str;
  int   size;

  if (str = Lib_malloc(size = (strlen(s) + 1)))
    memcpy(str, s, size);
  return str;
}


/* Debug stuff -------------------------------------------------------------------------------- */

#ifdef DEBUG
void Heap_MemDump(char *message, void *addr, int size)
{
  char buffer[256];

  fprintf(stderr,"Memdump of %s:\n", message);

  while (size>0)
  {
    char *prt = buffer;
    int i;

    prt+=sprintf(prt, "%.8X : ", (int)addr);

    for (i=0;i<4;i++)
      prt+=sprintf(prt, "%.8X ", ((int*)addr)[i]);
    for (i=0;i<16;i++)
    {
      char b = ((char*)addr)[i];

      prt+=sprintf(prt, "%c", ((b&127)>=32 && b!=127)? b : 46);
    }

    fprintf(stderr,"%s\n", buffer);
    addr = (char*)addr + 16;
    size -= 16;
  }
}

void Heap_List(void *heap)
{
  int alive, total;
  void *wblk = NULL;
  int init = 1;

  fprintf(stderr, "--- List of claimed blocks in %s ---\n", heap_name(heap));
  while (_swix(MAlloc_EnumerateBlocks, _INR(1, 2) | _OUT(0) | _OUTR(2, 3), heap,
         wblk, &alive, &wblk, &total), wblk && total)
  {
    if (alive)
    {
      heap_info *info = (heap_info*) wblk;
      char *blk = (char*)wblk + IN_S;
      int ofs = (blk - (char*)heap)/1024;

      if (!info->initialisation)
        init = 0;

      if (!init)
        fprintf(stderr, "%4dK (%p): %8d bytes (%s line %ld)\n", ofs, blk, info->user_size,
                info->claim_file, info->claim_line);
    }
  }
  fprintf(stderr, "------------------------------\n");
}


void Heap_Dump(void *heap)
{
  int alive, total;
  void *wblk = NULL, *maxblk = heap;

  fprintf(stderr, "--- Dump of %s ---\n", heap_name(heap));
  while (_swix(MAlloc_EnumerateBlocks, _INR(1, 2) | _OUT(0) | _OUTR(2, 3), heap,
         wblk, &alive, &wblk, &total), wblk && total)
  {
    if (alive)
    {
      Heap_verify(heap, (char*)wblk + IN_S, NULL, 0, TRUE, FALSE);
      if (wblk > maxblk)
        maxblk = wblk;
    }
  }
  fprintf(stderr, "\nTotal potential waste: %d Kbytes\n", ((char*)maxblk - (char*)heap)/1024);
  fprintf(stderr, "----------------------------\n");
}


void Heap_Stats(void *heap)
{
  int alive, total;
  void *wblk = NULL;
  int free = 0;
  int freelargest = 0;
  int claimed = 0;
  int claimedlargest = 0;
  int freenum = 0;
  int claimednum = 0;

  if (!heap)
    return;

  fprintf(stderr, "--- %s status ---\n", heap_name(heap));
  if (heap != heap_libs)
    Heap_VerifyBlocks(heap);

  while (_swix(MAlloc_EnumerateBlocks, _INR(1, 2) | _OUT(0) | _OUTR(2, 3),
         heap, wblk, &alive, &wblk, &total), wblk && total)
  {
    if (alive)
    {
      claimed += total;
      claimednum++;
      if (total > claimedlargest)
        claimedlargest = total;
    }
    else
    {
      free += total;
      freenum++;
      if (total > freelargest)
        freelargest = total;
    }
  }
  fprintf(stderr, "Total claimed: %dK in %d blocks (largest = %d bytes)\n", claimed/1024, claimednum, claimedlargest);
  fprintf(stderr, "Total free:    %dK in %d blocks (largest = %d bytes)\n", free/1024, freenum, freelargest);
  fprintf(stderr, "Total:         %dK in %d blocks\n", (free+claimed)/1024, freenum+claimednum);
  fprintf(stderr, "----------------------------\n");
}


void Heap_VerifyBlocks(void *heap)
{
  int alive, total;
  void *wblk = NULL;

  if (!heap)
    return;

  while (_swix(MAlloc_EnumerateBlocks, _INR(1, 2) | _OUT(0) | _OUTR(2, 3), heap, wblk, &alive, &wblk, &total), wblk && total)
  {
    if (alive)
      Heap_verify(heap, (char*)wblk + IN_S, NULL, 0, FALSE, TRUE);
  }
}


void Heap_ListAll(void)
{
  heap_data *data = heaplist;

  while (data)
  {
    Heap_List(data->heap);
    fprintf(stderr, "\n");
    data = data->next;
  }
}


void Heap_DumpAll(void)
{
  heap_data *data = heaplist;

  while (data)
  {
    if (!data->no_dump)
    {
      Heap_Dump(data->heap);
      fprintf(stderr, "\n");
    }
    data = data->next;
  }
}


void Heap_StatsAll(void)
{
  heap_data *data = heaplist;
  int space;
  int num;
  char *name;

  Heap_Stats(heap_libs);
  fprintf(stderr, "\n");

  while (data)
  {
    Heap_Stats(data->heap);
    fprintf(stderr, "\n");
    data = data->next;
  }

  fprintf(stderr, "------ Dynamic areas -------\n");
  num = -1;
  do
  {
    if (_swix(OS_DynamicArea, _INR(0,1)|_OUT(1), 3, num, &num) || num == -1)
      break;

    if (_swix(OS_DynamicArea, _INR(0,1)|_OUT(2)|_OUT(8), 2, num, &space, &name))
      continue;

    fprintf(stderr,"%s = %dK\n", name, space/1024);

  } while (1);
  fprintf(stderr, "----------------------------\n");

  if (_swix(Wimp_SlotSize, _INR(0,1) | _OUT(2), -1, -1, &space) == NULL)
    fprintf(stderr,"Free space = %dK\n\n", space / 1024);

  _swix(0x107, 0);
}


void Heap_VerifyBlocksAll(void)
{
  heap_data *data = heaplist;

  while (data)
  {
    Heap_VerifyBlocks(data->heap);
    data = data->next;
  }
}



#endif


/* -------------------------------------------------------------------------------------------- */


/* Create a heap. It will be removed on exit.
 * Failure to create a heap is a fatal error.
 */
void *Heap_Create(const char *name, int level, int maxsize, void (*squeeze)(void))
{
  void *heap;
  heap_data *data;

  if (!_swix(MAlloc_Initialise, _INR(0, 4) | _OUT(0), name, 1<<3, 0, 4096, maxsize, &heap))
  {
    if (data = Lib_calloc(1, sizeof(heap_data)))
    {
      data->heap    = heap;
      data->level   = level & ~1;
      data->squeeze = squeeze;
      data->no_dump = level & 1;
      data->next = heaplist;
      heaplist = data;
      return heap;
    }
  }
  fprintf(stderr, "Could not create %s\n", name);
  abort();
  return NULL;
}



void Heap_SetLevel(void *heap, int level)
{
  heap_data *data = heaplist;

  while (data)
  {
    if (data->heap == heap)
    {
      data->level = level;
      return;
    }
    data = data->next;
  }
}



/* Called at exit to remove all heaps
 */
static void Heap_Finalise(void)
{
  heap_data *data = heaplist;

  while (data)
  {
    _swix(MAlloc_Finalise, _IN(1), data->heap);
    data = data->next;
  }
  _swix(MAlloc_Finalise, _IN(1), heap_libs);
}



/* Called at init to create libs and misc heaps
 */
void Heap_Initialise(void)
{
  if (!_swix(MAlloc_Initialise, _INR(0, 4) | _OUT(0), "JPEGView (Temp)", 1<<3, 0, 4096, 16*1024*1024, &heap_libs))
  {
    atexit(Heap_Finalise);
    //heap_misc = Heap_Create(_APPNAME_ " (Misc)", 4<<20, 32 * 1024 * 1024, Cache_Squeeze);
    return;
  }
  else
    fprintf(stderr, "Could not create libs heap\n");
  abort();
}



void Heap_InitialisationDone(void)
{
  heap_initialising = 0;
}



/* Called every 50cs to keep heaps below their max level
 */
int Heap_Shrink(void *vzs)
{
  int free;
  heap_data *data = heaplist;

  while (data)
  {
    if (data->squeeze && ((int*)data->heap)[1] > data->level)
      data->squeeze();
    _swix(MAlloc_Shrink, _IN(1), data->heap);
    data = data->next;
  }
  _swix(MAlloc_Shrink, _IN(1), heap_libs);

  _swi(Wimp_SlotSize, _INR(0,1)|_OUT(2), -1, -1, &free);

#ifdef HEAP_DEBUG
  Heap_VerifyBlocksAll();
#endif

  return 0;
}



