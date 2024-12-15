#include "base.h"

static void arena_block_deinit(ArenaBlock *b)
{
    if (!b) return;
    free(b->block);
    arena_block_deinit(b->next);
    free(b);
}

void arena_free(Arena *arena)
{
    arena_block_deinit(arena->first);
}

void *arena_alloc(Arena *arena, u64 size)
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

void arena_reset(Arena *arena)
{
    ArenaBlock *b = arena->first;
    while(b)
    {
        b->end = 0;
        b = b->next;
    }
    arena->current = arena->first;
}

string string_view_from_bytes(u8 *cstr, usize len)
{
    string s = {0};

    s.data = cstr;
    s.len = len;

    return s;
}

string string_from_bytes(Arena *arena, u8 *cstr, usize len)
{
    string s = {0};

    s.data = arena_alloc(arena, len);
    memmove(s.data, cstr, len);
    s.len = len;

    return s;
}