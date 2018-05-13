// flint_arena: A simple memory arena.
// Version: 1.0.0
// Dependencies: None
//
// VERSION HISTORY
////////////////////////////////////////////////////////////////////////////////
//
// v1.0.0 (2018-05-13)
//      Initial release
// 
// INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////
//
// To use, write "#define FLINT_ARENA_IMPL" in one of your C files before
// including this file.
// 
// EXAMPLE
////////////////////////////////////////////////////////////////////////////////
#if 0

#define FLINT_ARENA_IMPL
#include "flint_arena.h"

// ...

void * alloc_fn(size_t size)
{
    void * ptr = malloc(size);
    printf("Allocating %10.d %p\n", (int)size, ptr);
    return ptr;
}

void free_fn(void * ptr)
{
    printf("Freeing block %p\n", ptr);
    free(ptr);
}

// ...

FlArena arena;
fl_arena_init(&arena, alloc_fn, free_fn, 1024 * 1024);
void * my_ptr_1 = fl_arena_alloc(&arena, 200);
void * my_ptr_2 = fl_arena_alloc(&arena, 1234);
fl_arena_free(&arena);
void * my_ptr_3 = fl_arena_alloc(&arena, 1000);
fl_arena_free(&arena);

#endif
// LICENCE
////////////////////////////////////////////////////////////////////////////////
// 
// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org/>

#ifndef FLINT_ARENA_DECLS
#define FLINT_ARENA_DECLS

typedef void * (* FlArenaAllocFn)(size_t);
typedef void (* FlArenaFreeFn)(void *);

typedef struct FlArena {
    char **         blocks;
    char *          current;
    size_t          size_left;
    FlArenaAllocFn  alloc_fn;
    FlArenaFreeFn   free_fn;
    size_t          block_size;
} FlArena;

// Initializes the memory arena.
//
// arena is the FlArena to be allocated.
// All previous states are ignored.
//
// alloc_fn is the memory allocation callback.
// Prototype is void *(size_t size).
// Behavior is undefined if returns NULL.
//
// free_fn is the memory freeing callback.
// Prototype is void(void * ptr).
//
// block_size is the minimum size of the blocks allocated by
// the arena. If a memory region larger than this size is requested,
// a larger block will be allocated.
void fl_arena_init(
        FlArena * arena,
        FlArenaAllocFn alloc_fn,
        FlArenaFreeFn free_fn,
        size_t block_size);

// Allocates a memory region from the arena.
void * fl_arena_alloc(FlArena * arena, size_t size);

// Frees all blocks allocated by the arena.
// All memory regions previously allocated by fl_arena_alloc
// are subsequently invalid.
void fl_arena_free(FlArena * arena);

#endif // Decls

#ifdef FLINT_ARENA_IMPL

enum {
    BLOCK_OVERHEAD = sizeof(char *)
};

void fl_arena_init(
        FlArena * arena,
        FlArenaAllocFn alloc_fn,
        FlArenaFreeFn free_fn,
        size_t block_size)
{
    arena->blocks = 0;
    arena->current = 0;
    arena->size_left = 0;
    arena->alloc_fn = alloc_fn;
    arena->free_fn = free_fn;
    arena->block_size = block_size;
}

void * fl_arena_alloc(
        FlArena * arena,
        size_t size)
{
    if (arena->size_left < size) {
        size_t block_size = (size < arena->block_size)
            ? arena->block_size
            : size;
        block_size;

        size_t block_node_size = block_size + BLOCK_OVERHEAD;
        char ** new_block_node = (char **)arena->alloc_fn(block_node_size);
        if (!new_block_node) {
            return 0;
        }

        *new_block_node = (char *)arena->blocks;
        arena->blocks = new_block_node;

        char * new_block = (char *)new_block_node + BLOCK_OVERHEAD;
        arena->current = new_block;
        arena->size_left = block_size;
    }

    void * allocated = arena->current;
    arena->size_left -= size;
    arena->current += size;
    return allocated;
}

void fl_arena_free(FlArena * arena)
{
    char ** block = arena->blocks;
    while (block) {
        char * next_block = *block;
        arena->free_fn(block);
        block = (char **)next_block;
    }

    arena->blocks = 0;
    arena->current = 0;
    arena->size_left = 0;
}

#endif // Impl
