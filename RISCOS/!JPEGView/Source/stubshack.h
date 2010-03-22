#ifndef __StubsHack_h
#define __StubsHack_h

#include <stdlib.h>

#ifndef BOOL
#define BOOL  unsigned
#define FALSE 0
#define TRUE  1
#endif


typedef void (*StubsHack_fnptr)( void);
	/* A generic pointer to a function.	*/

typedef void *(*StubsHack_mallocfn)	( size_t);
typedef void *(*StubsHack_reallocfn)	( void *, size_t);
typedef void *(*StubsHack_callocfn)	( size_t, size_t);
typedef void  (*StubsHack_freefn)	( void *);
	/* Some function-pointer types...	*/


extern StubsHack_mallocfn	StubsHack_scl_malloc;
extern StubsHack_freefn		StubsHack_scl_free;
extern StubsHack_reallocfn	StubsHack_scl_realloc;
extern StubsHack_callocfn	StubsHack_scl_calloc;
	/* These will always point to the vanilla shared c lib functions.	*/
	/* They will initially point to malloc etc in stubs. After a succesful	*/
	/* call to StubsHack_ReplaceANSIAllocFns, they will be modified to 	*/
	/* point directly to the functions in the shared c library, because the	*/
	/* stubs functions will point elsewhere...				*/


typedef enum	{
	StubsHack_error_OK = 0,
	StubsHack_error_NOTSTUBS
	}
	StubsHack_error;


StubsHack_error	StubsHack_RedirectStubsFn(
		StubsHack_fnptr	stubs_fn,
		StubsHack_fnptr	replacement_fn,
		StubsHack_fnptr	*old_fn_store
		);
	/*
	Replaces a single stubs function 'stubs_fn'. All subsequent calls to
	'stubs_fn' go to 'replacement_fn'. The actual SharedCLibrary function
	which implements 'stubs_fn' is put into '*old_fn_store' (if this isn't
	NULL), which you should use if 'replacement_fn' is just a wrapper function.
	The redirection will not take place if 'replacement_fn' is NULL, but 
	'old_fn_store ' will still be set.
	
	Most stubs functions aren't 'void foo( void)', so you will probably
	need to cast the first two arguments to this function to type
	(StubsHack_fnptr), and the last to (StubsHack_fnptr *).
	*/



StubsHack_error	StubsHack_ReplaceANSIAllocFns(
		StubsHack_mallocfn	new_malloc,
		StubsHack_reallocfn	new_realloc,
		StubsHack_callocfn	new_calloc,
		StubsHack_freefn	new_free,

		StubsHack_mallocfn	*old_malloc_store,
		StubsHack_reallocfn	*old_realloc_store,
		StubsHack_callocfn	*old_calloc_store,
		StubsHack_freefn	*old_free_store,

		BOOL			make_stack_allocater_be_mallocfree
		);
/*

Allows total replacement of the ANSI heap management functions. The
first four parameters are your replacement functions. (Use NULL to leave
a stubs function unchanged.). The next four params are addresses of
pointers to functions which, if not NULL, will be filled in to point to
the previous re/c/malloc/free functions. Thus your 'new_malloc'
functions could call 'old_malloc_store' after doing some error-checking
etc, making it a 'wrapper' function.

The first time 'StubsHack_ReplaceANSIAllocFns' is called, it sets the
global variables StubsHack_scl_malloc/realloc/calloc/free to point to
the original destination of malloc/etc. This enables you to always have
access to the vanilla SharedCLib functions, and also enables the use of
_kernel_register_allocs as follows:

If 'make_stack_allocaters_be_mallocfree' is TRUE,
_kernel_register_allocs will be called with the addresses of malloc/free
in the shared c library (using StubsHack_scl_malloc etc), so that stack
chunks will be allocated using the standard C malloc/free functions.
This is the best option, as the functions which allocate stack chunks
mustn't use more than 41 words of stack and mustn't check the stack.

If 'make_stack_allocaters_be_mallocfree' is FALSE, then
_kernel_register_allocs will not be called, so (assuming that
the stack allocators are malloc and free before
StubsHack_ReplaceANSIAllocFns is called) new_malloc and new_free will be
used to allocate stack henceforth, which is only ok if they are
functions which don't check the stack and use less than 41 words of
stack.

The only problem I can forsee is if malloc was used before this
function, in which case your 'new_free' function will be called with
a pointer that was allocated using the shared c library malloc,
rather than your 'new_malloc' function. Hence call
StubsHack_ReplaceANSIAllocFns asap in your prog.

Note that 'StubsHack_ReplaceANSIAllocFns' can be called repeatedly,
diverting malloc to different functions. When malloc is actually used,
these functions will call each other in a nested fasion, as long as each
one uses what it thinks is the original malloc function - ie. non of
them actually replace malloc completely.


*/








/* The stuff below here is probably not much use...	*/
/* 'tis used internally though.				*/







StubsHack_fnptr	StubsHack_GetDestOfB( StubsHack_fnptr address);
	/* Returns the address in the shared c lib of the function 'address'	*/
	/* I think this will only work properly when the branch is a forward	*/
	/* one. (ok for branches from stubs into the shared c lib		*/
	/* Returns NULL if the function isn't a stubs function, ie. isn't a 	*/
	/* simple ARM branch instruction.					*/





#define StubsHack_MakeBranchInstruction( instr, dest)				\
	*((int *) ((int) (instr))) = 						\
		0xEA000000 							\
		|								\
		(								\
			((((int) (dest)) - ((int) (instr)) - 8) / 4)		\
			& 0x00FFFFFF						\
		)								\
	/* Makes the word at address 'instr' be a simple branch to address	*/
	/* 'destination'							*/
	/* i.e. top byte is 0xEA, and bottom 3 bytes are offset (in words) of	*/
	/* 'dest' from 'instr'.							*/
	/* This is a macro 'cos we don't want too much chance of stack 		*/
	/* extensions (caused by fn calls) while malloc etc are being changed.	*/
	/* Actually, _kernel_register_allocs is called now so we should be ok.	*/




#endif
