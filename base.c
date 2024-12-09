#include "base.h"

static void ArenaBlockDeinit(ArenaBlock *b) 
{
    if (!b) return;
    free(b->block);
    ArenaBlockDeinit(b->next);
    free(b);
}

void ArenaFree(Arena *arena)
{
    ArenaBlockDeinit(arena->first);
}

void *ArenaAlloc(Arena *arena, u64 size)
{
    if ((!arena->first) || (!arena->current))
    {
        arena->first = arena->current = malloc(sizeof(ArenaBlock));
        u64 size = BASE_BLOCK_SIZE;
        arena->first->block = malloc(size);
        arena->first->size = size;
        arena->first->end = 0;
        arena->first->next = NULL;
    }

    while (arena->current->end+size > arena->current->size)
    {
        if (!arena->current->next)
        {
            arena->current->next = malloc(sizeof(ArenaBlock));
            
            u64 s = (size > BASE_BLOCK_SIZE) ? size : BASE_BLOCK_SIZE;
            arena->current->next->block = malloc(s);
            arena->current->next->size = s;
            arena->current->next->end = 0;
            arena->current->next->next = NULL;
        }
        arena->current = arena->current->next;
    }
    
    void *ptr = arena->current->block+arena->current->end;
    arena->current->end += size;
    
    return ptr;
}

void ArenaReset(Arena *arena)
{
    ArenaBlock *b = arena->first;
    while(b)
    {
        b->end = 0;
        b = b->next;
    }
    arena->current = arena->first;
}

String StringViewFromBytes(u8 *cstr, usize len)
{
    String s = {0};
    
    s.data = cstr;
    s.len = len;
    
    return s;
}

String StringFromBytes(Arena *arena, u8 *cstr, usize len)
{
    String s = {0};
    
    s.data = ArenaAlloc(arena, len);
    memmove(s.data, cstr, len);
    s.len = len;
    
    return s;
}